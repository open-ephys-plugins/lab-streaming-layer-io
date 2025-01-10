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

#ifndef LSLINLETTHREAD_H_DEFINED
#define LSLINLETTHREAD_H_DEFINED

#include <DataThreadHeaders.h>

#include <lsl_cpp.h>

const float DEFAULT_DATA_SCALE = 1.0f;
const int STREAM_SELECTION_UNDEFINED = -1;
const double TIMESTAMP_UNDEFINED = -1;
const size_t DEFAULT_NUM_SAMPLES = 1024;

class LSLInletThread : public DataThread
{
public:
    /** The class constructor, used to initialize any members. */
    LSLInletThread (SourceNode* sn);

    /** The class destructor, used to deallocate memory */
    ~LSLInletThread();

    /** User defined scaling factor for samples */
    float dataScale;

    /** Number of samples to read during each update cycle */
    int numSamples;

    /** Index (in the discovered streams list) of the user selected LSL stream */
    int selectedDataStream;

    /** Index (in the discovered streams list) of the user selected LSL stream that will be used to read markers */
    int selectedMarkersStream;

    /** List of most recently discovered LSL streams */
    std::vector<lsl::stream_info> availableStreams;

    /** Organize available streams into data and marker streams */
    std::vector<lsl::stream_info> dataStreams;
    std::vector<lsl::stream_info> markerStreams;

    /** Perform LSL stream discovery */
    void discover();

    /** Define a file path that can be used to read the TTL mapping for the markers stream.
     * The content of the file should be a name-value pair in json format, for example:
     * {
     *   "Marker_1": 1,
     *   "Marker_2": 2
     * }
     */
    bool setMarkersMappingPath (std::string filePath);

    // ------------------------------------------------------------
    //                  PURE VIRTUAL METHODS
    //     (must be implemented by all DataThreads)
    // ------------------------------------------------------------

    /** Called repeatedly to add any available data to the buffer */
    bool updateBuffer() override;

    /** Returns true if the data source is connected, false otherwise.*/
    bool foundInputSource() override;

    /** Initializes data transfer.*/
    bool startAcquisition() override;

    /** Stops data transfer.*/
    bool stopAcquisition() override;

    /* Passes the processor's info objects to DataThread, to allow them to be configured */
    void updateSettings (OwnedArray<ContinuousChannel>* continuousChannels,
                         OwnedArray<EventChannel>* eventChannels,
                         OwnedArray<SpikeChannel>* spikeChannels,
                         OwnedArray<DataStream>* sourceStreams,
                         OwnedArray<DeviceInfo>* devices,
                         OwnedArray<ConfigurationObject>* configurationObjects) override;

    // ------------------------------------------------------------
    //                   VIRTUAL METHODS
    //       (can optionally be overriden by sub-classes)
    // ------------------------------------------------------------

    /** Create the DataThread custom editor */
    std::unique_ptr<GenericEditor> createEditor (SourceNode* sn) override;

    /** Register parameters */
    void registerParameters() override;

    /** Handle parameter changes */
    void parameterValueChanged (Parameter* param) override;

    // ** Allows the DataThread plugin to respond to messages sent by other processors */
    void handleBroadcastMessage (const String& msg, const int64 messageTimeMilliseconds) override;

    // ** Allows the DataThread plugin to handle a config message while acquisition is not active. */
    String handleConfigMessage (const String& msg) override;

private:
    template <typename T>
    void printBuffer (const char* desc, T* buf, size_t size);
    void readMarkers (std::size_t samples_to_read);
    std::string trim (const std::string str);

    int64 totalSamples;
    lsl::stream_inlet* dataStream;
    lsl::stream_inlet* markersStream;

    float* dataBuffer;
    float* samples;
    double* timestampBuffer;

    int64* sampleNumbers;
    uint64* ttlEventWords;
    std::map<std::string, uint64> eventMap;

    int numChannels;
    double initialTimestamp;

    bool firstConnect = true;
};

#endif
