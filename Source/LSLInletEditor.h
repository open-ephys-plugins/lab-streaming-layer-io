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

#ifndef LSLINLETEDITOR_H_DEFINED
#define LSLINLETEDITOR_H_DEFINED

#include <EditorHeaders.h>
#include "LSLInletThread.h"

class RefreshButton : public Button
{

public:
    /** Constructor */
    RefreshButton ();

    /** Destructor */
    ~RefreshButton() {}

    void paintButton (Graphics& g, bool isMouseOver, bool isButtonDown) override;

    /** Sets the button bounds when editor is resized */
    void parentSizeChanged() override;

private:

    std::unique_ptr<Drawable> refreshIcon;
};

class LSLInletEditor : public GenericEditor,
					   public Button::Listener,
					   public ComboBox::Listener,
					   public Label::Listener
{
public:
	/** The class constructor, used to initialize any members. */
	LSLInletEditor(GenericProcessor *parentNode, LSLInletThread *inlet);

	/** The class destructor, used to deallocate memory */
	~LSLInletEditor() {}

	/** Button listener callback, called by button when pressed. */
	void buttonClicked(Button *button) override;

	/** Called when a ComboBox has its selected item changed. */
	void comboBoxChanged(ComboBox *comboBoxThatHasChanged) override;

	/** Called when label is changed */
	void labelTextChanged(Label *label) override;

	/** Called by processor graph in beginning of the acqusition, disables editor completly. */
	void startAcquisition() override;

	/** Called by processor graph at the end of the acqusition, reenables editor completly. */
	void stopAcquisition() override;

	/** Called when configuration is saved. Adds editors config to xml. */
	void saveCustomParametersToXml(XmlElement *xml) override;

	/** Called when configuration is loaded. Reads editors config from xml. */
	void loadCustomParametersFromXml(XmlElement *xml) override;

private:
	File lastFilePath;

	// Button that tried to connect to client
	//ScopedPointer<UtilityButton> discoverButton;
	std::unique_ptr<RefreshButton> refreshButton;

	// Data stream selection
	ScopedPointer<Label> dataStreamSelectorLabel;
	ScopedPointer<ComboBox> dataStreamSelectorBox;

	// Marker stream selection
	ScopedPointer<Label> markerStreamSelectorLabel;
	ScopedPointer<ComboBox> markerStreamSelectorBox;

	// Marker stream mapping
	ScopedPointer<UtilityButton> fileButton;
	ScopedPointer<Label> markerStreamMappingLabel;
	ScopedPointer<Label> fileNameLabel;

	// Scaling
	ScopedPointer<Label> scaleLabel;
	ScopedPointer<Label> scaleInput;

	// Parent node
	LSLInletThread *inletThread;
};

#endif
