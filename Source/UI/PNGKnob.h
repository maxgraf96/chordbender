//
// Created by Max on 31/10/2023.
//

#ifndef CHORDBENDER_PNGKNOB_H
#define CHORDBENDER_PNGKNOB_H

#include <JuceHeader.h>

class PNGKnob : public juce::Component
{
public:
    PNGKnob(const char* imageData, size_t dataSize, int width, int height);
    void setValue(float value); // 0.0f to 1.0f

    void paint(Graphics& g) override;
    void resized() override;

    void mouseDown(const MouseEvent& event) override;
    void mouseDrag(const MouseEvent& event) override;

    float KNOB_MIN_VALUE = 100.0f;
    float KNOB_MAX_VALUE = 5000.0f;

private:
    juce::Image knobImage;
    float currentValue = 0.0f; // Range: 0.0f to 1.0f
    AffineTransform getRotationTransform() const;

    Point<float> dragStartPos;
    float dragStartValue = 0.0f;

    std::unique_ptr<Label> nameLabel;
    std::unique_ptr<Label> valueLabel;
};


#endif //CHORDBENDER_PNGKNOB_H
