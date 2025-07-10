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

RefreshButton::RefreshButton() : Button ("Refresh")
{
    XmlDocument xmlDoc (R"(
        <svg width="800px" height="800px" viewBox="0 0 24 24" fill="none" xmlns="http://www.w3.org/2000/svg">
<path d="M13 2L11 3.99545L11.0592 4.05474M11 18.0001L13 19.9108L12.9703 19.9417M11.0592 4.05474L13 6M11.0592 4.05474C11.3677 4.01859 11.6817 4 12 4C16.4183 4 20 7.58172 20 12C20 14.5264 18.8289 16.7793 17 18.2454M7 5.75463C5.17107 7.22075 4 9.47362 4 12C4 16.4183 7.58172 20 12 20C12.3284 20 12.6523 19.9802 12.9703 19.9417M11 22.0001L12.9703 19.9417" stroke="#000000" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"/>
</svg>
    )");

    refreshIcon = Drawable::createFromSVG (*xmlDoc.getDocumentElement().get());

    setClickingTogglesState (false);
}

void RefreshButton::paintButton (Graphics& g, bool isMouseOver, bool isButtonDown)
{
    Colour buttonColour = Colours::darkgrey;

    if (isMouseOver && isEnabled())
        buttonColour = Colours::yellow;

    refreshIcon->replaceColour (Colours::black, buttonColour);

    refreshIcon->drawWithin (g, getLocalBounds().toFloat(), RectanglePlacement::centred, 1.0f);

    refreshIcon->replaceColour (buttonColour, Colours::black);
}

void RefreshButton::parentSizeChanged()
{
    setBounds (getParentWidth() - 25, 4, 16, 16);
}

LSLInletEditor::LSLInletEditor(GenericProcessor *parentNode, LSLInletThread *thread)
    : GenericEditor(parentNode)
{
    inletThread = thread;

    desiredWidth = 200;

    // Stream selector
    addSelectedStreamParameterEditor(Parameter::PROCESSOR_SCOPE, "data_stream", 10, 29);
    addTextBoxParameterEditor(Parameter::PROCESSOR_SCOPE, "scale", 10, 54);
    addSelectedStreamParameterEditor(Parameter::PROCESSOR_SCOPE, "marker_stream", 10, 79);
    addPathParameterEditor(Parameter::PROCESSOR_SCOPE, "mapping", 10, 104);

    refreshButton = std::make_unique<RefreshButton>();
    refreshButton->setBounds (desiredWidth - 65, 4, 16, 16);
    refreshButton->addListener (this);
    refreshButton->setTooltip ("Re-scan network for available LSL streams");
    addChildComponent (refreshButton.get());
    refreshButton->setVisible (true);
}

void LSLInletEditor::buttonClicked(Button *button)
{
    if (button == refreshButton.get())
    {
        inletThread->discover();
    }
}

void LSLInletEditor::startAcquisition()
{
    refreshButton->setEnabled (false);
}

void LSLInletEditor::stopAcquisition()
{
    refreshButton->setEnabled (true);
}