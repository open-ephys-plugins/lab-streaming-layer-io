/*
 ------------------------------------------------------------------

 This file is part of the Open Ephys GUI
 Copyright (C) 2022 Open Ephys

 ------------------------------------------------------------------

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */

#include "LSLInletThread.h"
#include "LSLInletEditor.h"

#include <fstream>
#include <sstream>

LSLInletThread::LSLInletThread(SourceNode *sn) : DataThread(sn),
                                                 numSamples(DEFAULT_NUM_SAMPLES),
                                                 dataScale(DEFAULT_DATA_SCALE),
                                                 selectedDataStream(STREAM_SELECTION_UNDEFINED)
{
    numChannels = 1; // start with 1 channel, will resize the buffers as needed
    // calculate data buffer size instead of using a fixed value if the function of (numChannels, numSamples) can be determined
    sourceBuffers.add(new DataBuffer(numChannels, 100000));
    this->dataStream = NULL;
    this->markersStream = NULL;

    dataBuffer = (float *)malloc(numChannels * numSamples * sizeof(float));
    timestampBuffer = (double *)malloc(numSamples * sizeof(double));

    sampleNumbers = (int64 *)malloc(numSamples * sizeof(int64));
    ttlEventWords = (uint64 *)malloc(numSamples * sizeof(uint64));

    eventMap["0"] = 0;
    eventMap["1"] = 1;
    eventMap["2"] = 2;
    eventMap["3"] = 3;
    eventMap["4"] = 4;
    eventMap["5"] = 5;
    eventMap["6"] = 6;
    eventMap["7"] = 7;
    eventMap["8"] = 8;
}

LSLInletThread::~LSLInletThread()
{
    free(dataBuffer);
    free(timestampBuffer);
    free(sampleNumbers);
    free(ttlEventWords);
}

void LSLInletThread::discover()
{
    availableStreams = lsl::resolve_streams(1.0);
}

bool LSLInletThread::updateBuffer()
{
    int max_samples_total = numChannels * numSamples;
    std::size_t multiplexed_samples_read = 0;

    try
    {
        multiplexed_samples_read = dataStream->pull_chunk_multiplexed(dataBuffer, timestampBuffer, max_samples_total, numSamples);
    }
    catch (const std::runtime_error &re)
    {
        LOGE("Failed to read data samples with runtime error: ", re.what());
    }
    catch (const std::exception &ex)
    {
        LOGE("Failed to read data samples with runtime exception: ", ex.what());
    }

    if (multiplexed_samples_read <= 0)
    {
        // it probably doesn't make sense to read markers if we didn't find samples
        return true;
    }

    std::size_t data_samples_read = multiplexed_samples_read / numChannels;
    jassert(multiplexed_samples_read % numChannels == 0);

    readMarkers(data_samples_read);

    if (initialTimestamp == TIMESTAMP_UNDEFINED)
    {
        initialTimestamp = timestampBuffer[0];
    }

    // Compute sample numbers and
    // convert time from absolute to relative
    for (int i = 0; i < data_samples_read; i++)
    {
        sampleNumbers[i] = totalSamples + i;
        timestampBuffer[i] -= initialTimestamp;
    }

    // Apply user defined scaling
    for (int i = 0; i < multiplexed_samples_read; i++)
    {
        dataBuffer[i] = (float)(dataScale * dataBuffer[i]);
    }

    sourceBuffers[0]->addToBuffer(
        dataBuffer,
        sampleNumbers,
        timestampBuffer,
        ttlEventWords,
        (int)data_samples_read,
        1);

    totalSamples += data_samples_read;

    return true;
}

// expects timestampBuffer to be populated with sample timestamps
// in order to match markers with data samples
void LSLInletThread::readMarkers(std::size_t samples_to_read)
{
    if (markersStream == NULL)
    {
        return;
    }

    // clear TTL buffer
    for (int i = 0; i < samples_to_read; i++)
    {
        ttlEventWords[i] = 0;
    }

    try
    {
        std::string sample;
        int i = 0;
        while (double ts = markersStream->pull_sample(&sample, 1, 0.0))
        {
            // broadcast the marker text to the signal chain
            broadcastMessage(sample);

            // try to find closest event timestamp among sample timestamps
            bool found_match = false;
            for (int j = i; samples_to_read; j++)
            {
                if (timestampBuffer[j] >= ts)
                {
                    i = j;
                    found_match = true;
                    break;
                }
            }
            if (!found_match)
            {
                LOGE("Discarding marker because it couldn't be matched with data sample timestamp");
                break;
            }

            if (eventMap.find(sample) == eventMap.end())
            {
                LOGC("Mapping not found for marker ", sample);
            }
            else
            {
                ttlEventWords[0] = 1ULL << (eventMap[sample] - 1);
            }

            if (++i >= samples_to_read)
            {
                break;
            }
        }
    }
    catch (const std::runtime_error &re)
    {
        LOGE("Failed to read markers with runtime error: ", re.what());
    }
    catch (const std::exception &ex)
    {
        LOGE("Failed to read markers with exception: ", ex.what());
    }
}

bool LSLInletThread::foundInputSource()
{
    return true;
}

bool LSLInletThread::startAcquisition()
{
    if (selectedDataStream == STREAM_SELECTION_UNDEFINED)
    {
        LOGC("Not starting acquisition because no data stream was selected");
        return false;
    }

    totalSamples = 0;
    initialTimestamp = TIMESTAMP_UNDEFINED;

    this->dataStream = new lsl::stream_inlet(availableStreams[selectedDataStream]);

    numChannels = availableStreams[selectedDataStream].channel_count();
    sourceBuffers[0]->resize(numChannels, 100000);

    if (auto newBuffer = (float *)realloc(dataBuffer, numChannels * numSamples * sizeof(float)))
    {
        dataBuffer = newBuffer;
    }
    else
    {
        LOGE("Failed to allocate data buffer");
        return false;
    }
    if (auto newBuffer = (double *)realloc(timestampBuffer, numSamples * sizeof(double)))
    {
        timestampBuffer = newBuffer;
    }
    else
    {
        LOGE("Failed to allocate timestamp buffer");
        return false;
    }

    if (auto newBuffer = (int64 *)realloc(sampleNumbers, numSamples * sizeof(int64)))
    {
        sampleNumbers = newBuffer;
    }
    else
    {
        LOGE("Failed to allocate sample numbers buffer");
        return false;
    }
    if (auto newBuffer = (uint64 *)realloc(ttlEventWords, numSamples * sizeof(uint64)))
    {
        ttlEventWords = newBuffer;

        // initialize the TTL buffer to 0
        for (int i = 0; i < numSamples; i++)
        {
            ttlEventWords[i] = 0;
        }
    }
    else
    {
        LOGE("Failed to allocate TTL word buffer");
        return false;
    }

    if (selectedMarkersStream != STREAM_SELECTION_UNDEFINED)
    {
        this->markersStream = new lsl::stream_inlet(availableStreams[selectedMarkersStream]);
        jassert(this->markersStream->get_channel_count() == 1);
    }

    startThread();
    return true;
}

bool LSLInletThread::stopAcquisition()
{
    if (isThreadRunning())
    {
        signalThreadShouldExit();
    }

    waitForThreadToExit(500);

    if (this->dataStream != NULL)
    {
        this->dataStream->close_stream();
        delete this->dataStream;
        this->dataStream = NULL;
    }
    if (this->markersStream != NULL)
    {
        this->markersStream->close_stream();
        delete this->markersStream;
        this->markersStream = NULL;
    }

    sourceBuffers[0]->clear();
    return true;
}

void LSLInletThread::updateSettings(OwnedArray<ContinuousChannel> *continuousChannels,
                                    OwnedArray<EventChannel> *eventChannels,
                                    OwnedArray<SpikeChannel> *spikeChannels,
                                    OwnedArray<DataStream> *sourceStreams,
                                    OwnedArray<DeviceInfo> *devices,
                                    OwnedArray<ConfigurationObject> *configurationObjects)
{
    continuousChannels->clear();
    eventChannels->clear();
    devices->clear();
    spikeChannels->clear();
    configurationObjects->clear();
    sourceStreams->clear();

    if (selectedDataStream == STREAM_SELECTION_UNDEFINED)
    {
        return;
    }

    numChannels = availableStreams[selectedDataStream].channel_count();

    DataStream::Settings settings{
        availableStreams[selectedDataStream].name(),
        availableStreams[selectedDataStream].type(),
        availableStreams[selectedDataStream].source_id(),

        (float)availableStreams[selectedDataStream].nominal_srate()

    };
    sourceStreams->add(new DataStream(settings));
    for (int ch = 0; ch < availableStreams[selectedDataStream].channel_count(); ch++)
    {
        ContinuousChannel::Settings settings{
            ContinuousChannel::Type::ELECTRODE,
            "CH" + String(ch + 1),
            "description",
            "identifier",

            0.195f,

            sourceStreams->getFirst()};

        continuousChannels->add(new ContinuousChannel(settings));
    }

    EventChannel::Settings eventSettings{
        EventChannel::Type::TTL,
        "Events" + availableStreams[selectedDataStream].source_id(),
        "description",
        "identifier",
        sourceStreams->getFirst(),
        64};

    const auto ec = new EventChannel(eventSettings);

    eventChannels->add(ec);
}

std::unique_ptr<GenericEditor> LSLInletThread::createEditor(SourceNode *sn)
{
    std::unique_ptr<LSLInletEditor> editor = std::make_unique<LSLInletEditor>(sn, this);
    return editor;
}

void LSLInletThread::handleBroadcastMessage(String msg)
{
}

String LSLInletThread::handleConfigMessage(String msg)
{
    return "";
}

bool LSLInletThread::setMarkersMappingPath(std::string filePath)
{
    eventMap.clear();
    std::ifstream file(filePath);

    if (!file)
    {
        LOGE("Cannot open file: ", filePath);
        return false;
    }

    // Perform a naive JSON parsing
    try
    {
        std::stringstream ss;
        ss << file.rdbuf();
        file.close();

        // remove curly braces
        std::string str = ss.str();
        str.erase(std::remove(str.begin(), str.end(), '{'), str.end());
        str.erase(std::remove(str.begin(), str.end(), '}'), str.end());

        // split by comma
        std::vector<std::string> pairs;
        size_t pos = 0;
        std::string token;
        while ((pos = str.find(',')) != std::string::npos)
        {
            pairs.push_back(str.substr(0, pos));
            str.erase(0, pos + 1);
        }
        pairs.push_back(str);

        // update mapping for TTL
        for (const auto pair : pairs)
        {
            if ((pos = pair.find(':')) != std::string::npos)
            {
                std::string marker = trim(pair.substr(0, pos));
                uint64 ttl = std::stoul(trim(pair.substr(pos + 1, pair.size())));
                eventMap[marker] = ttl;
                LOGC("Saved mapping for marker: ", marker, "=", ttl);
            }
        }
    }
    catch (const std::runtime_error &re)
    {
        LOGE("Failed to read markers mapping file with runtime error: ", re.what());
        return false;
    }
    catch (const std::exception &ex)
    {
        LOGE("Failed to read markers mapping file with runtime exception: ", ex.what());
        return false;
    }

    return true;
}

inline std::string LSLInletThread::trim(const std::string const_str)
{
    std::string str = const_str;
    str.erase(str.find_last_not_of(" \n\r\t\"") + 1);
    str.erase(0, str.find_first_not_of(" \n\r\t\""));
    return str;
}
