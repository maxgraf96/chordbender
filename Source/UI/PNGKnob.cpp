//
// Created by Max on 31/10/2023.
//

#include "PNGKnob.h"

PNGKnob::PNGKnob(const char *imageData, size_t dataSize, int w, int h) {
    knobImage = ImageCache::getFromMemory(imageData, dataSize).rescaled(w, h);
    setInterceptsMouseClicks(true, false); // This component should receive mouse events, but not its child components (if any)

    // Configure and add the name label
    nameLabel = std::make_unique<Label>("nameLabel", "Bend Duration");
    nameLabel->setJustificationType(Justification::centred);
    nameLabel->setColour(Label::textColourId, Colours::white);
    nameLabel->setBounds(0, 0, 100, 20);
    addAndMakeVisible(*nameLabel);

    // Configure and add the value label
    valueLabel = std::make_unique<Label>("valueLabel", "0");
    valueLabel->setJustificationType(Justification::centred);
    valueLabel->setColour(Label::textColourId, Colours::white);
    valueLabel->setBounds(0, 120, 100, 20);
    addAndMakeVisible(*valueLabel);
}

void PNGKnob::setValue(float value) {
    currentValue = jlimit(KNOB_MIN_VALUE, KNOB_MAX_VALUE, value);
    valueLabel->setText(String((int) currentValue), dontSendNotification);
    repaint();
}

void PNGKnob::paint(Graphics &g) {
    g.drawImageTransformed(knobImage, getRotationTransform());
}

void PNGKnob::resized() {
//    FlexBox flexBox;
//    flexBox.flexDirection = FlexBox::Direction::column;
//    flexBox.justifyContent = FlexBox::JustifyContent::center;
//    flexBox.alignItems = FlexBox::AlignItems::center;
//
//    // Add the name label to the flexbox
//    FlexItem nameLabelItem(nameLabel->getWidth(), nameLabel->getHeight());
//    nameLabelItem.associatedComponent = nameLabel.get();
//
//    // Add the knob (PNG image) to the flexbox
//    FlexItem knobImageItem(getWidth(), knobImage.getHeight());
//
//    // Add the value label to the flexbox
//    FlexItem valueLabelItem(valueLabel->getWidth(), valueLabel->getHeight());
//    valueLabelItem.associatedComponent = valueLabel.get();
//
//    flexBox.items.addArray({ nameLabelItem, knobImageItem, valueLabelItem });
//
//    flexBox.performLayout(getLocalBounds());
}

AffineTransform PNGKnob::getRotationTransform() const {
    float normalizedValue = (currentValue - KNOB_MIN_VALUE) / (KNOB_MAX_VALUE - KNOB_MIN_VALUE);
    // Calculate the rotation angle. For this example, we assume a rotation from 0 to 270 degrees.
    float rotationAngle = MathConstants<float>::pi * (-120.0f + 240.0f * normalizedValue) / 180.0f; // Convert degrees to radians

    // Calculate the center of the image
    float centerX = knobImage.getWidth() * 0.5f;
    float centerY = knobImage.getHeight() * 0.5f;

    // Create a rotation transform around the image center
    return AffineTransform::rotation(rotationAngle, centerX, centerY).followedBy(AffineTransform::translation(20.0f, 40.0f));
}


void PNGKnob::mouseDown(const MouseEvent& event)
{
    dragStartPos = event.position;
    dragStartValue = currentValue;
}

void PNGKnob::mouseDrag(const MouseEvent& event)
{
    float dragDistance = dragStartPos.y - event.position.y;
    float newValue = dragStartValue + dragDistance * 10.0f; // The factor 0.005f controls the sensitivity of the drag
    setValue(newValue);
}
