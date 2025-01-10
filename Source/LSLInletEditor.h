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

#include "LSLInletThread.h"
#include <EditorHeaders.h>

class RefreshButton : public Button
{
public:
    /** Constructor */
    RefreshButton();

    /** Destructor */
    ~RefreshButton() {}

    void paintButton (Graphics& g, bool isMouseOver, bool isButtonDown) override;

    /** Sets the button bounds when editor is resized */
    void parentSizeChanged() override;

private:
    std::unique_ptr<Drawable> refreshIcon;
};

class LSLInletEditor : public GenericEditor,
                       public Button::Listener
{
public:
    /** The class constructor, used to initialize any members. */
    LSLInletEditor (GenericProcessor* parentNode, LSLInletThread* inlet);

    /** The class destructor, used to deallocate memory */
    ~LSLInletEditor() {}

    /** Button listener callback, called by button when pressed. */
    void buttonClicked (Button* button) override;

    /** Updates editor state on start of acquisition */
    void startAcquisition() override;

    /** Updates editor state on stop of acquisition */
    void stopAcquisition() override;

private:
    std::unique_ptr<RefreshButton> refreshButton;

    LSLInletThread* inletThread;
};

#endif
