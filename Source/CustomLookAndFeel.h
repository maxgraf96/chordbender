//
// Created by Max on 23/10/2023.
//

#ifndef CHORD_BENDER_CUSTOMLOOKANDFEEL_H
#define CHORD_BENDER_CUSTOMLOOKANDFEEL_H

#include <JuceHeader.h>

class CustomLookAndFeel : public juce::LookAndFeel_V4
{
public:
    void drawScrollbar (Graphics &g, ScrollBar & scrollbar,
                        int x, int y,
                        int width, int height,
                        bool isScrollbarVertical,
                        int thumbStartPosition, int thumbSize,
                        bool isMouseOver, bool isMouseDown)  override
    {
        juce::LookAndFeel_V4::drawScrollbar(g, scrollbar, x, y, width, height,
                                            isScrollbarVertical,
                                            thumbStartPosition, thumbSize,
                                            isMouseOver, isMouseDown);
        // Set custom color for the scrollbar thumb
        g.setColour(juce::Colours::lightgrey);
        if (isScrollbarVertical)
            g.fillRect(x, thumbStartPosition, width, thumbSize);
        else
            g.fillRect(thumbStartPosition, y, thumbSize, height);
    }
};


#endif //CHORD_BENDER_CUSTOMLOOKANDFEEL_H
