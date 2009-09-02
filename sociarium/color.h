// s.o.c.i.a.r.i.u.m
// HASHIMOTO, Yasuhiro (E-mail: hy @ sys.t.u-tokyo.ac.jp)
// update: 2009/03/06

/* Copyright (c) 2005-2009, HASHIMOTO, Yasuhiro, All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 *   - Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *
 *   - Redistributions in binary form must reproduce the above copyright notice,
 *     this list of conditions and the following disclaimer in the documentation
 *     and/or other materials provided with the distribution.
 *
 *   - Neither the name of the University of Tokyo nor the names of its contributors
 *     may be used to endorse or promote products derived from this software without
 *     specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef INCLUDE_GUARD_SOCIARIUM_PROJECT_COLOR_H
#define INCLUDE_GUARD_SOCIARIUM_PROJECT_COLOR_H

namespace hashimoto_ut {

  namespace sociarium_project_color {

    enum {
      BACKGROUND = 0,
      LAYOUT_FRAME_BORDER,
      LAYOUT_FRAME_AREA,
      GRID,
      TIME_SLIDER_UPPER_ON,
      TIME_SLIDER_UPPER_OFF,
      TIME_SLIDER_LOWER_ON,
      TIME_SLIDER_LOWER_OFF,
      MOUSE_DRAG_REGION,
      SELECTION_FRAME,
      THREAD_MESSAGE_FRAME,
      LAYER_LABEL_FRAME,
      SIZE
    };

    namespace {
      float const select[SIZE][4] = {
        { 0.0f, 0.0f, 0.0f, 0.0f },  // BACKGROUND
				//{ 1.0f, 1.0f, 1.0f, 0.0f },  // BACKGROUND
        { 1.0f, 1.0f, 1.0f, 0.4f },  // LAYOUT_FRAME_BORDER
        { 1.0f, 1.0f, 0.9f, 0.3f },  // LAYOUT_FRAME_AREA
        { 1.0f, 1.0f, 1.0f, 0.5f },  // GRID
        { 1.0f, 1.0f, 1.0f, 0.7f },  // TIME_SLIDER_UPPER_ON
        { 1.0f, 1.0f, 1.0f, 0.3f },  // TIME_SLIDER_UPPER_OFF
        { 0.2f, 0.2f, 1.0f, 1.0f },  // TIME_SLIDER_LOWER_ON
        { 0.2f, 0.2f, 1.0f, 0.5f },  // TIME_SLIDER_LOWER_OFF
        { 1.0f, 1.0f, 1.0f, 0.2f },  // MOUSE_DRAG_REGION
        { 0.1f, 0.1f, 0.1f, 0.6f },  // SELECTION_FRAME
        { 0.1f, 0.1f, 0.1f, 0.6f },  // THREAD_MESSAGE_FRAME
        { 0.1f, 0.1f, 0.1f, 0.6f },  // LAYER_LABEL_FRAME
      };
    }

  } // The end of the namespace "sociarium_project_color"

} // The end of the namespace "hashimoto_ut"

#endif // INCLUDE_GUARD_SOCIARIUM_PROJECT_COLOR_H
