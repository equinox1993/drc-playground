// Author: 2020, Yuwei Huang.
//
// A simple DRC demo that displays color on the Gamepad's screen. It does not
// require OpenGL, SDL, and whatnot.
//
// Control:
//   Button A: Cycle through all colors defined in RGB space.
//   Home button: Shutdown the gamepad and exit the app.
//   Left joystick left & right: Change blueness.
//   Left joystick top & bottom: Change greenness.
//   Right joystick left & right: Change redness.

#include <drc/input.h>
#include <drc/screen.h>
#include <drc/streamer.h>
#include <unistd.h>

#include <algorithm>
#include <cmath>
#include <iostream>
#include <vector>

enum ColorComponent {
  kCCBlue = 0,
  kCCGreen = 1,
  kCCRed = 2,
  kCCAlpha = 3,
};

uint8_t* ColorAsU8A(uint32_t* color_ptr) {
  return reinterpret_cast<uint8_t*>(color_ptr);
}

uint32_t ColorOf(uint8_t blue, uint8_t green, uint8_t red, uint8_t alpha = 0) {
  uint32_t val;
  uint8_t* val_u8a = ColorAsU8A(&val);
  val_u8a[kCCBlue] = blue;
  val_u8a[kCCGreen] = green;
  val_u8a[kCCRed] = red;
  val_u8a[kCCAlpha] = alpha;
  return val;
}

// Returns true if |current_color| is changed.
bool MoveTowards(uint32_t target_color, uint32_t* current_color) {
  uint8_t* target_color_u8a = ColorAsU8A(&target_color);
  uint8_t* current_color_u8a = ColorAsU8A(current_color);
  bool component_changed = false;

  for (int i = 0; i < 4; i++) {
    int16_t diff =
        static_cast<int16_t>(target_color_u8a[i]) - current_color_u8a[i];
    if (diff == 0) {
      continue;
    }
    if (diff > 0) {
      current_color_u8a[i] += 3;
      if (current_color_u8a[i] > target_color_u8a[i]) {
        current_color_u8a[i] = target_color_u8a[i];
      }
    } else {
      current_color_u8a[i] -= 3;
      if (current_color_u8a[i] < target_color_u8a[i]) {
        current_color_u8a[i] = target_color_u8a[i];
      }
    }
    component_changed = true;
  }

  return component_changed;
}

void MoveBy(ColorComponent component, float delta, uint32_t* current_color) {
  uint8_t* current_color_u8a = ColorAsU8A(current_color);
  float new_val = current_color_u8a[component] + delta;
  new_val = std::round(new_val);
  if (new_val < 0) {
    new_val = 0;
  } else if (new_val > 255) {
    new_val = 255;
  }
  current_color_u8a[component] = static_cast<uint8_t>(new_val);
}

void FillColor(std::vector<uint32_t>* frame, uint32_t color) {
  for (size_t i = 0; i < frame->size(); i++) {
    frame->data()[i] = color;
  }
}

int main(int argc, char const *argv[]) {
  std::cout << "Now running drc_helloworld..." << std::endl;

  drc::Streamer streamer;
  if (!streamer.Start()) {
    std::cerr << "Unable to start streamer" << std::endl;
    return 1;
  }

  std::cout << "Screen size: " << drc::kScreenWidth << " * "
            << drc::kScreenHeight << std::endl;

  std::vector<uint32_t> frame;
  frame.reserve(drc::kScreenWidth * drc::kScreenHeight);

  uint32_t current_color = ColorOf(255, 0, 0);
  uint32_t target_colors[8] = {
    ColorOf(255, 0, 0),
    ColorOf(0, 255, 0),
    ColorOf(255, 255, 0),
    ColorOf(0, 0, 255),
    ColorOf(255, 0, 255),
    ColorOf(0, 255, 255),
    ColorOf(255, 255, 255),
    ColorOf(0, 0, 0),
  };
  size_t current_target = 0u;

  for (size_t i = 0u; i < drc::kScreenWidth * drc::kScreenHeight; i++) {
    frame.push_back(current_color);
  }

  // Audio doesn't work for some reason...
  // std::vector<drc::s16> audio_samples(48000 * 2);
  // for (int i = 0; i < 48000; ++i) {
  //   float t = i / (48000.0 - 1);
  //   drc::s16 samp = (drc::s16)(32000 * sinf(2 * M_PI * t * 480));
  //   audio_samples[2 * i] = samp;
  //   audio_samples[2 * i + 1] = samp;
  // }

  while (true) {
    drc::InputData input_data;
    streamer.PollInput(&input_data);
    if (input_data.valid && input_data.buttons & drc::InputData::kBtnHome) {
      break;
    }

    bool is_color_changed = true;

    if (input_data.valid && input_data.buttons & drc::InputData::kBtnA &&
        !MoveTowards(target_colors[current_target], &current_color)) {
      // Change target color on button release.
      current_target = (current_target + 1) % 8;
      is_color_changed = false;
    } else {
      if (input_data.valid && input_data.left_stick_x != 0) {
        MoveBy(kCCBlue, 5.0 * input_data.left_stick_x, &current_color);
      }
      if (input_data.valid && input_data.left_stick_y != 0) {
        MoveBy(kCCGreen, 5.0 * input_data.left_stick_y, &current_color);
      }
      if (input_data.valid && input_data.right_stick_x != 0) {
        MoveBy(kCCRed, 5.0 * input_data.right_stick_x, &current_color);
      }
    }

    if (is_color_changed) {
      FillColor(&frame, current_color);
    }

    drc::u8* frame_data_begin = ColorAsU8A(frame.data());
    std::vector<drc::u8> frame_out(
        frame_data_begin, 
        frame_data_begin + frame.size() * 4);
    streamer.PushVidFrame(&frame_out, drc::kScreenWidth, drc::kScreenHeight,
      drc::PixelFormat::kBGRA);
    // streamer.PushAudSamples(audio_samples);
    usleep(16666);  // 60fps
  }

  streamer.ShutdownPad();
  usleep(100000);  // 100ms

  streamer.Stop();
  return 0;
}
