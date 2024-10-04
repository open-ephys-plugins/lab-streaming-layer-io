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

#include "LSLInletEditor.h"

LSLInletEditor::LSLInletEditor(GenericProcessor *parentNode, LSLInletThread *thread)
    : GenericEditor(parentNode)
{
    lastFilePath = CoreServices::getDefaultUserSaveDirectory();
    inletThread = thread;

    desiredWidth = 220;

    // Add connect button
    discoverButton = new UtilityButton("Refresh streams");
    discoverButton->setRadius(3.0f);
    discoverButton->setBounds(10, 35, 100, 20);
    discoverButton->addListener(this);
    addAndMakeVisible(discoverButton);

    // Data stream combo box
    dataStreamSelectorLabel = new Label("Select a data stream", "Select a data stream");
    dataStreamSelectorLabel->setFont(Font("Small Text", 10, Font::plain));
    dataStreamSelectorLabel->setBounds(10, 60, 160, 8);
    dataStreamSelectorLabel->setColour(Label::textColourId, Colours::darkgrey);
    addAndMakeVisible(dataStreamSelectorLabel);

    dataStreamSelectorBox = new ComboBox();
    dataStreamSelectorBox->setBounds(10, 70, 200, 20);
    dataStreamSelectorBox->addListener(this);
    addAndMakeVisible(dataStreamSelectorBox);

    // Markers stream combo box
    markerStreamSelectorLabel = new Label("Select a marker stream", "Select a marker stream");
    markerStreamSelectorLabel->setFont(Font("Small Text", 10, Font::plain));
    markerStreamSelectorLabel->setBounds(10, 95, 100, 8);
    markerStreamSelectorLabel->setColour(Label::textColourId, Colours::darkgrey);
    addAndMakeVisible(markerStreamSelectorLabel);

    markerStreamSelectorBox = new ComboBox();
    markerStreamSelectorBox->setBounds(10, 105, 100, 20);
    markerStreamSelectorBox->addListener(this);
    addAndMakeVisible(markerStreamSelectorBox);

    // Markers stream mapping
    markerStreamMappingLabel = new Label("Marker mapping file", "Marker mapping file");
    markerStreamMappingLabel->setFont(Font("Small Text", 10, Font::plain));
    markerStreamMappingLabel->setBounds(110, 95, 100, 8);
    markerStreamMappingLabel->setColour(Label::textColourId, Colours::darkgrey);
    addAndMakeVisible(markerStreamMappingLabel);

    fileButton = new UtilityButton("F:");
    fileButton->setRadius(3.0f);
    fileButton->setBounds(115, 105, 20, 20);
    fileButton->addListener(this);
    addAndMakeVisible(fileButton);

    fileNameLabel = new Label("Selected file", "No file selected");
    fileNameLabel->setFont(Font("Small Text", 10, Font::plain));
    fileNameLabel->setBounds(140, 105, 70, 20);
    fileNameLabel->setEditable(true);
    fileNameLabel->setEnabled(false);
    fileNameLabel->setColour(Label::backgroundColourId, Colours::lightgrey);
    fileNameLabel->addListener(this);
    addAndMakeVisible(fileNameLabel);

    // Scale
    scaleLabel = new Label("Scale", "Scale");
    scaleLabel->setFont(Font("Small Text", 10, Font::plain));
    scaleLabel->setBounds(120, 26, 40, 8);
    scaleLabel->setColour(Label::textColourId, Colours::darkgrey);
    addAndMakeVisible(scaleLabel);

    scaleInput = new Label("Scale", String(inletThread->dataScale));
    scaleInput->setFont(Font("Small Text", 10, Font::plain));
    scaleInput->setBounds(120, 37, 40, 16);
    scaleInput->setEditable(true);
    scaleInput->setColour(Label::backgroundColourId, Colours::lightgrey);
    scaleInput->addListener(this);
    addAndMakeVisible(scaleInput);
}

void LSLInletEditor::startAcquisition()
{
    // Disable the whole GUI
    discoverButton->setEnabled(false);
    streamSelector->setEnabled(false);
    fileButton->setEnabled(false);
    dataStreamSelectorBox->setEnabled(false);
    markerStreamSelectorBox->setEnabled(false);
}

void LSLInletEditor::stopAcquisition()
{
    // Reenable the whole GUI
    discoverButton->setEnabled(true);
    streamSelector->setEnabled(true);
    fileButton->setEnabled(true);
    dataStreamSelectorBox->setEnabled(true);
    markerStreamSelectorBox->setEnabled(true);
}

// Button::Listener
void LSLInletEditor::buttonClicked(Button *button)
{
    if (button == discoverButton)
    {
        dataStreamSelectorBox->clear();
        markerStreamSelectorBox->clear();
        inletThread->discover();

        int selectedDataStreamIndex = STREAM_SELECTION_UNDEFINED;
        int selectedMarkerStreamIndex = 0;
        markerStreamSelectorBox->addItem("None", STREAM_SELECTION_UNDEFINED);

        for (int i = 0; i < inletThread->availableStreams.size(); i++)
        {
            const auto &s = inletThread->availableStreams[i];
            if (s.nominal_srate() > 0)
            {
                dataStreamSelectorBox->addItem(s.name() + " (" + s.type() + ")", i + 1);
                selectedDataStreamIndex = 0;
            }
            else
            {
                if (s.channel_count() != 1)
                {
                    LOGC("Skipping irregular stream ", s.name(), " because it doesn't have exactly 1 channel.\n", s.as_xml());
                    continue;
                }
                markerStreamSelectorBox->addItem(s.name() + " (" + s.type() + ")", i + 1);
            }
        }

        dataStreamSelectorBox->setSelectedItemIndex(selectedDataStreamIndex);
        inletThread->selectedDataStream = selectedDataStreamIndex;
        markerStreamSelectorBox->setSelectedItemIndex(selectedMarkerStreamIndex);
        inletThread->selectedMarkersStream = STREAM_SELECTION_UNDEFINED;

        CoreServices::updateSignalChain(this);
    }
    else if (button == fileButton)
    {
        String supportedFormats = "*.json";

        FileChooser chooseFileReaderFile("Please select a json file containing the markers mapping...",
                                         lastFilePath,
                                         supportedFormats);

        if (chooseFileReaderFile.browseForFileToOpen())
        {
            if (inletThread->setMarkersMappingPath(chooseFileReaderFile.getResult().getFullPathName().toStdString()))
            {
                fileNameLabel->setText(chooseFileReaderFile.getResult().getFileName(), dontSendNotification);
            }
        }
    }
}

// ComboBox::Listener
void LSLInletEditor::comboBoxChanged(ComboBox *box)
{
    if (box == dataStreamSelectorBox)
    {
        inletThread->selectedDataStream = box->getSelectedId() - 1;

        inletThread->resizeBuffers();

        CoreServices::updateSignalChain(this);
    }
    else if (box == markerStreamSelectorBox)
    {
        if (box->getSelectedId() == STREAM_SELECTION_UNDEFINED)
        {
            inletThread->selectedMarkersStream = STREAM_SELECTION_UNDEFINED;
        }
        else
        {
            inletThread->selectedMarkersStream = box->getSelectedId() - 1;
        }
        CoreServices::updateSignalChain(this);
    }
}

// Label::Listener
void LSLInletEditor::labelTextChanged(Label *label)
{
    if (label == scaleInput)
    {
        float scale = scaleInput->getText().getFloatValue();
        if (scale > 0.0f && scale <= 10000.0f)
        {
            inletThread->dataScale = scale;
        }
        else
        {
            scaleInput->setText(String(inletThread->dataScale), dontSendNotification);
        }
    }
}

void LSLInletEditor::saveCustomParametersToXml(XmlElement *xmlNode)
{
    XmlElement *parameters = xmlNode->createNewChildElement("PARAMETERS");

    parameters->setAttribute("scale", scaleInput->getText());
}

void LSLInletEditor::loadCustomParametersFromXml(XmlElement *xmlNode)
{
    forEachXmlChildElement(*xmlNode, subNode)
    {
        if (subNode->hasTagName("PARAMETERS"))
        {
            scaleInput->setText(subNode->getStringAttribute("scale", String(DEFAULT_DATA_SCALE)), dontSendNotification);
            inletThread->dataScale = subNode->getDoubleAttribute("scale", DEFAULT_DATA_SCALE);
        }
    }
}
