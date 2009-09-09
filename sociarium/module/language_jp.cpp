// s.o.c.i.a.r.i.u.m: module/language_jp.cpp
// HASHIMOTO, Yasuhiro (E-mail: hy @ sys.t.u-tokyo.ac.jp)

/* Copyright (c) 2005-2009, HASHIMOTO, Yasuhiro, All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *   - Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   - Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   - Neither the name of the University of Tokyo nor the names of its
 *     contributors may be used to endorse or promote products derived from
 *     this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <cassert>
#include <string>
#include <vector>
#include <unordered_map>
#include <windows.h>
#include "../resource.h"
#include "../language.h"

BOOL WINAPI DllMain (HINSTANCE hinstDll, DWORD fdwReason, LPVOID lpvReserved) {
  return TRUE;
}

namespace hashimoto_ut {

  using std::vector;
  using std::wstring;
  using std::tr1::unordered_map;

  extern "C" __declspec(dllexport)
    void __cdecl load_menu(unordered_map<int, wstring>& menu) {

      /////////////////////////////////////////////////////////////////////////////
      menu[IDM_FILE]
        = L"�t�@�C��(&F)";
      menu[IDM_FILE_CANCEL]
        = L"�L�����Z��(&C)\tESC";
      menu[IDM_FILE_QUIT]
        = L"�I��(&X)\tESC";


      /////////////////////////////////////////////////////////////////////////////
      menu[IDM_VIEW]
        = L"�`��(&V)";
      menu[IDM_VIEW_SHOW_NODE]
        = L"�m�[�h��\��(&1)\t1";
      menu[IDM_VIEW_SHOW_EDGE]
        = L"�G�b�W��\��(&2)\t2";
      menu[IDM_VIEW_SHOW_COMMUNITY]
        = L"�R�~���j�e�B��\��(&3)\t3";
      menu[IDM_VIEW_SHOW_COMMUNITY_EDGE]
        = L"�R�~���j�e�B�G�b�W��\��(&4)\t4";

      menu[IDM_VIEW_NODE_SIZE]
        = L"�m�[�h�̑傫��(&N)";
      menu[IDM_VIEW_NODE_SIZE_UPDATE]
        = L"�Čv�Z(&0)";
      menu[IDM_VIEW_NODE_SIZE_CANCEL]
        = L"�L�����Z��(&C)";
      menu[IDM_VIEW_NODE_SIZE_UNIFORM]
        = L"��l(&1)";
      menu[IDM_VIEW_NODE_SIZE_WEIGHT]
        = L"�d��(&2)";
      menu[IDM_VIEW_NODE_SIZE_DEGREE_CENTRALITY]
        = L"�������S��(&3)";
      menu[IDM_VIEW_NODE_SIZE_CLOSENESS_CENTRALITY]
        = L"�������S��(&4)";
      menu[IDM_VIEW_NODE_SIZE_BETWEENNESS_CENTRALITY]
        = L"�}��S��(&5)";
      menu[IDM_VIEW_NODE_SIZE_PAGERANK]
        = L"�y�[�W�����N(&6)";
      menu[IDM_VIEW_NODE_SIZE_POINT]
        = L"�_(&7)";

      menu[IDM_VIEW_EDGE_WIDTH]
        = L"�G�b�W�̑���(&E)";
      menu[IDM_VIEW_EDGE_WIDTH_UPDATE]
        = L"�Čv�Z(&0)";
      menu[IDM_VIEW_EDGE_WIDTH_CANCEL]
        = L"�L�����Z��(&C)";
      menu[IDM_VIEW_EDGE_WIDTH_UNIFORM]
        = L"��l(&1)";
      menu[IDM_VIEW_EDGE_WIDTH_WEIGHT]
        = L"�d��(&2)";
      menu[IDM_VIEW_EDGE_WIDTH_BETWEENNESS_CENTRALITY]
        = L"�}��S��(&3)";

      menu[IDM_VIEW_NODE_STYLE]
        = L"�m�[�h�̕`����@(&5)\tShift+Ctrl+1";
      menu[IDM_VIEW_NODE_STYLE_POLYGON]
        = L"�|���S��(&1)";
      menu[IDM_VIEW_NODE_STYLE_TEXTURE]
        = L"�e�N�X�`��(&2)";
      menu[IDM_VIEW_EDGE_STYLE]
        = L"�G�b�W�̕`����@(&6)\tShift+Ctrl+2";
      menu[IDM_VIEW_EDGE_STYLE_LINE]
        = L"���C��(&1)";
      menu[IDM_VIEW_EDGE_STYLE_POLYGON]
        = L"�|���S��(&2)";
      menu[IDM_VIEW_COMMUNITY_STYLE]
        = L"�R�~���j�e�B�̕`����@(&7)\tShift+Ctrl+3";
      menu[IDM_VIEW_COMMUNITY_STYLE_POLYGON_CIRCLE]
        = L"�|���S��(&1)";
      menu[IDM_VIEW_COMMUNITY_STYLE_TEXTURE]
        = L"�e�N�X�`��(&2)";
      menu[IDM_VIEW_COMMUNITY_EDGE_STYLE]
        = L"�R�~���j�e�B�G�b�W�̕`����@(&8)\tShift+Ctrl+4";
      menu[IDM_VIEW_COMMUNITY_EDGE_STYLE_LINE]
        = L"���C��(&1)";
      menu[IDM_VIEW_COMMUNITY_EDGE_STYLE_POLYGON]
        = L"�|���S��(&2)";


      /////////////////////////////////////////////////////////////////////////////
      menu[IDM_STRING]
        = L"������(&S)";
      menu[IDM_STRING_SHOW_NODE_NAME]
        = L"�m�[�h�̖��O��\��(&1)\tCtrl+1";
      menu[IDM_STRING_SHOW_EDGE_NAME]
        = L"�G�b�W�̖��O��\��(&2)\tCtrl+2";
      menu[IDM_STRING_SHOW_COMMUNITY_NAME]
        = L"�R�~���j�e�B�̖��O��\��(&3)\tCtrl+3";
      menu[IDM_STRING_SHOW_COMMUNITY_EDGE_NAME]
        = L"�R�~���j�e�B�G�b�W�̖��O��\��(&4)\tCtrl+4";
      menu[IDM_STRING_NODE_NAME_SIZE]
        = L"�m�[�h�̖��O�̑傫��(&5)";
      menu[IDM_STRING_NODE_NAME_SIZE_VARIABLE]
        = L"�m�[�h�̑傫���ɘA��(&0)";
      menu[IDM_STRING_NODE_NAME_SIZE_0]
        = L"�ŏ�(&1)";
      menu[IDM_STRING_NODE_NAME_SIZE_1]
        = L"��(&2)";
      menu[IDM_STRING_NODE_NAME_SIZE_2]
        = L"�W��(&3)";
      menu[IDM_STRING_NODE_NAME_SIZE_3]
        = L"��(&4)";
      menu[IDM_STRING_NODE_NAME_SIZE_4]
        = L"�ő�(&5)";
      menu[IDM_STRING_EDGE_NAME_SIZE]
        = L"�G�b�W�̖��O�̑傫��(&6)";
      menu[IDM_STRING_EDGE_NAME_SIZE_VARIABLE]
        = L"�G�b�W�̑����ɘA��(&0)";
      menu[IDM_STRING_EDGE_NAME_SIZE_0]
        = L"�ŏ�(&1)";
      menu[IDM_STRING_EDGE_NAME_SIZE_1]
        = L"��(&2)";
      menu[IDM_STRING_EDGE_NAME_SIZE_2]
        = L"�W��(&3)";
      menu[IDM_STRING_EDGE_NAME_SIZE_3]
        = L"��(&4)";
      menu[IDM_STRING_EDGE_NAME_SIZE_4]
        = L"�ő�(&5)";
      menu[IDM_STRING_COMMUNITY_NAME_SIZE]
        = L"�R�~���j�e�B�̖��O�̑傫��(&7)";
      menu[IDM_STRING_COMMUNITY_NAME_SIZE_VARIABLE]
        = L"�R�~���j�e�B�̑傫���ɘA��(&0)";
      menu[IDM_STRING_COMMUNITY_NAME_SIZE_0]
        = L"�ŏ�(&1)";
      menu[IDM_STRING_COMMUNITY_NAME_SIZE_1]
        = L"��(&2)";
      menu[IDM_STRING_COMMUNITY_NAME_SIZE_2]
        = L"�W��(&3)";
      menu[IDM_STRING_COMMUNITY_NAME_SIZE_3]
        = L"��(&4)";
      menu[IDM_STRING_COMMUNITY_NAME_SIZE_4]
        = L"�ő�(&5)";
      menu[IDM_STRING_COMMUNITY_EDGE_NAME_SIZE]
        = L"�R�~���j�e�B�G�b�W�̖��O�̑傫��(&8)";
      menu[IDM_STRING_COMMUNITY_EDGE_NAME_SIZE_VARIABLE]
        = L"�R�~���j�e�B�G�b�W�̑����ɘA��(&0)";
      menu[IDM_STRING_COMMUNITY_EDGE_NAME_SIZE_0]
        = L"�ŏ�(&1)";
      menu[IDM_STRING_COMMUNITY_EDGE_NAME_SIZE_1]
        = L"��(&2)";
      menu[IDM_STRING_COMMUNITY_EDGE_NAME_SIZE_2]
        = L"�W��(&3)";
      menu[IDM_STRING_COMMUNITY_EDGE_NAME_SIZE_3]
        = L"��(&4)";
      menu[IDM_STRING_COMMUNITY_EDGE_NAME_SIZE_4]
        = L"�ő�(&5)";
      menu[IDM_STRING_FONT_TYPE]
        = L"�����̕`����@(&R)";
      menu[IDM_STRING_FONT_TYPE_POLYGON]
        = L"�|���S��(&1)";
      menu[IDM_STRING_FONT_TYPE_TEXTURE]
        = L"�e�N�X�`��(&2)";
      menu[IDM_STRING_SHOW_FPS]
        = L"FPS��\��(&F)\tCtrl+F";
      menu[IDM_STRING_SHOW_LAYER_NAME]
        = L"���C���[�̖��O��\��(&L)\tL";


      /////////////////////////////////////////////////////////////////////////////
      menu[IDM_EDIT]
        = L"�ҏW(&E)";
      menu[IDM_EDIT_MARK_ON_CURRENT_LAYER]
        = L"���݂̃��C���[����I��(&1)";
      menu[IDM_EDIT_MARK_NODES_ON_CURRENT_LAYER]
        = L"�m�[�h��I��(&1)";
      menu[IDM_EDIT_MARK_EDGES_ON_CURRENT_LAYER]
        = L"�G�b�W��I��(&2)";
      menu[IDM_EDIT_MARK_NODES_INSIDE_COMMUNITY_ON_CURRENT_LAYER]
        = L"�R�~���j�e�B�ɏ�������m�[�h��I��(&3)";
      menu[IDM_EDIT_MARK_EDGES_INSIDE_COMMUNITY_ON_CURRENT_LAYER]
        = L"�R�~���j�e�B�ɏ�������G�b�W��I��(&4)";
      menu[IDM_EDIT_MARK_ELEMENTS_INSIDE_COMMUNITY_ON_CURRENT_LAYER]
        = L"�R�~���j�e�B�ɏ�������m�[�h�ƃG�b�W��I��(&5)";
      menu[IDM_EDIT_MARK_NODES_OUTSIDE_COMMUNITY_ON_CURRENT_LAYER]
        = L"�R�~���j�e�B�ɏ������Ȃ��m�[�h��I��(&6)";
      menu[IDM_EDIT_MARK_EDGES_OUTSIDE_COMMUNITY_ON_CURRENT_LAYER]
        = L"�R�~���j�e�B�ɏ������Ȃ��G�b�W��I��(&7)";
      menu[IDM_EDIT_MARK_ELEMENTS_OUTSIDE_COMMUNITY_ON_CURRENT_LAYER]
        = L"�R�~���j�e�B�ɏ������Ȃ��m�[�h�ƃG�b�W��I��(&8)";
      menu[IDM_EDIT_MARK_ALL_ELEMENTS_ON_CURRENT_LAYER]
        = L"�S�Ă�I��(&A)\tCtrl+A";
      menu[IDM_EDIT_INVERT_MARK_ON_CURRENT_LAYER]
        = L"�I���𔽓](&I)\tCtrl+I";

      menu[IDM_EDIT_MARK_ON_EACH_LAYER]
        = L"�S�Ẵ��C���[����I��(&2)";
      menu[IDM_EDIT_MARK_NODES_ON_EACH_LAYER]
        = L"�m�[�h��I��(&1)";
      menu[IDM_EDIT_MARK_EDGES_ON_EACH_LAYER]
        = L"�G�b�W��I��(&2)";
      menu[IDM_EDIT_MARK_NODES_INSIDE_COMMUNITY_ON_EACH_LAYER]
        = L"�R�~���j�e�B�ɏ�������m�[�h��I��(&3)";
      menu[IDM_EDIT_MARK_EDGES_INSIDE_COMMUNITY_ON_EACH_LAYER]
        = L"�R�~���j�e�B�ɏ�������G�b�W��I��(&4)";
      menu[IDM_EDIT_MARK_ELEMENTS_INSIDE_COMMUNITY_ON_EACH_LAYER]
        = L"�R�~���j�e�B�ɏ�������m�[�h�ƃG�b�W��I��(&5)";
      menu[IDM_EDIT_MARK_NODES_OUTSIDE_COMMUNITY_ON_EACH_LAYER]
        = L"�R�~���j�e�B�ɏ������Ȃ��m�[�h��I��(&6)";
      menu[IDM_EDIT_MARK_EDGES_OUTSIDE_COMMUNITY_ON_EACH_LAYER]
        = L"�R�~���j�e�B�ɏ������Ȃ��G�b�W��I��(&7)";
      menu[IDM_EDIT_MARK_ELEMENTS_OUTSIDE_COMMUNITY_ON_EACH_LAYER]
        = L"�R�~���j�e�B�ɏ������Ȃ��m�[�h�ƃG�b�W��I��(&8)";
      menu[IDM_EDIT_MARK_ALL_ELEMENTS_ON_EACH_LAYER]
        = L"�S�Ă�I��(&A)\tShift+Ctrl+A";
      menu[IDM_EDIT_INVERT_MARK_ON_EACH_LAYER]
        = L"�I���𔽓](&I)\tShift+Ctrl+I";

      menu[IDM_EDIT_HIDE_MARKED_ELEMENTS]
        = L"�I�����ꂽ�v�f���B��(&3)",
      menu[IDM_EDIT_HIDE_MARKED_NODES_ON_CURRENT_LAYER]
        = L"���݂̃��C���[�̑I�����ꂽ�m�[�h��Ώ�(&N)";
      menu[IDM_EDIT_HIDE_MARKED_EDGES_ON_CURRENT_LAYER]
        = L"���݂̃��C���[�̑I�����ꂽ�G�b�W��Ώ�(&E)";
      menu[IDM_EDIT_HIDE_MARKED_ELEMENTS_ON_CURRENT_LAYER]
        = L"���݂̃��C���[�̑I�����ꂽ�m�[�h�ƃG�b�W��Ώ�(&A)\tCtrl+H";
      menu[IDM_EDIT_HIDE_MARKED_NODES_ON_EACH_LAYER]
        = L"�S�Ẵ��C���[�̑I�����ꂽ�m�[�h��Ώ�(&N)";
      menu[IDM_EDIT_HIDE_MARKED_EDGES_ON_EACH_LAYER]
        = L"�S�Ẵ��C���[�̑I�����ꂽ�G�b�W��Ώ�(&E)";
      menu[IDM_EDIT_HIDE_MARKED_ELEMENTS_ON_EACH_LAYER]
        = L"�S�Ẵ��C���[�̑I�����ꂽ�m�[�h�ƃG�b�W��Ώ�(&A)\tShift+Ctrl+H";

      menu[IDM_EDIT_HIDE_MARKED_ELEMENTS_ALL]
        = L"�I�����ꂽ�v�f�̓���v�f��S�Ẵ��C���[����B��(&4)";
      menu[IDM_EDIT_HIDE_MARKED_NODES_ON_CURRENT_LAYER_ALL]
        = L"���݂̃��C���[�̑I�����ꂽ�m�[�h��Ώ�(&N)";
      menu[IDM_EDIT_HIDE_MARKED_EDGES_ON_CURRENT_LAYER_ALL]
        = L"���݂̃��C���[�̑I�����ꂽ�G�b�W��Ώ�(&E)";
      menu[IDM_EDIT_HIDE_MARKED_ELEMENTS_ON_CURRENT_LAYER_ALL]
        = L"���݂̃��C���[�̑I�����ꂽ�m�[�h�ƃG�b�W��Ώ�(&A)";
      menu[IDM_EDIT_HIDE_MARKED_NODES_ON_EACH_LAYER_ALL]
        = L"�S�Ẵ��C���[�̑I�����ꂽ�m�[�h��Ώ�(&N)";
      menu[IDM_EDIT_HIDE_MARKED_EDGES_ON_EACH_LAYER_ALL]
        = L"�S�Ẵ��C���[�̑I�����ꂽ�G�b�W��Ώ�(&E)";
      menu[IDM_EDIT_HIDE_MARKED_ELEMENTS_ON_EACH_LAYER_ALL]
        = L"�S�Ẵ��C���[�̑I�����ꂽ�m�[�h�ƃG�b�W��Ώ�(&A)";

      menu[IDM_EDIT_SHOW_HIDDEN_ELEMENTS]
        = L"�B�ꂽ�v�f��\��(&5)";
      menu[IDM_EDIT_SHOW_HIDDEN_NODES_ON_CURRENT_LAYER]
        = L"���݂̃��C���[�̉B�ꂽ�m�[�h��Ώ�(&N)";
      menu[IDM_EDIT_SHOW_HIDDEN_EDGES_ON_CURRENT_LAYER]
        = L"���݂̃��C���[�̉B�ꂽ�G�b�W��Ώ�(&E)";
      menu[IDM_EDIT_SHOW_HIDDEN_ELEMENTS_ON_CURRENT_LAYER]
        = L"���݂̃��C���[�̉B�ꂽ�m�[�h�ƃG�b�W��Ώ�(&A)\tCtrl+U";
      menu[IDM_EDIT_SHOW_HIDDEN_NODES_ON_EACH_LAYER]
        = L"�S�Ẵ��C���[�̉B�ꂽ�m�[�h��Ώ�(&N)";
      menu[IDM_EDIT_SHOW_HIDDEN_EDGES_ON_EACH_LAYER]
        = L"�S�Ẵ��C���[�̉B�ꂽ�G�b�W��Ώ�(&E)";
      menu[IDM_EDIT_SHOW_HIDDEN_ELEMENTS_ON_EACH_LAYER]
        = L"�S�Ẵ��C���[�̉B�ꂽ�m�[�h�ƃG�b�W��Ώ�(&A)\tShift+Ctrl+U";

      menu[IDM_EDIT_REMOVE_MARKED_ELEMENTS]
        = L"�I�����ꂽ�v�f���폜(&6)";
      menu[IDM_EDIT_REMOVE_MARKED_NODES_ON_CURRENT_LAYER]
        = L"���݂̃��C���[�̑I�����ꂽ�m�[�h��Ώ�(&N)";
      menu[IDM_EDIT_REMOVE_MARKED_EDGES_ON_CURRENT_LAYER]
        = L"���݂̃��C���[�̑I�����ꂽ�G�b�W��Ώ�(&E)";
      menu[IDM_EDIT_REMOVE_MARKED_ELEMENTS_ON_CURRENT_LAYER]
        = L"���݂̃��C���[�̑I�����ꂽ�m�[�h�ƃG�b�W��Ώ�(&A)\tDel";
      menu[IDM_EDIT_REMOVE_MARKED_NODES_ON_EACH_LAYER]
        = L"�S�Ẵ��C���[�̑I�����ꂽ�m�[�h��Ώ�(&N)";
      menu[IDM_EDIT_REMOVE_MARKED_EDGES_ON_EACH_LAYER]
        = L"�S�Ẵ��C���[�̑I�����ꂽ�G�b�W��Ώ�(&E)";
      menu[IDM_EDIT_REMOVE_MARKED_ELEMENTS_ON_EACH_LAYER]
        = L"�S�Ẵ��C���[�̑I�����ꂽ�m�[�h�ƃG�b�W��Ώ�(&A)\tCtrl+Del";

      menu[IDM_EDIT_REMOVE_MARKED_ELEMENTS_ALL]
        = L"�I�����ꂽ�v�f�̓���v�f��S�Ẵ��C���[����폜(&7)";
      menu[IDM_EDIT_REMOVE_MARKED_NODES_ON_CURRENT_LAYER_ALL]
        = L"���݂̃��C���[�̑I�����ꂽ�m�[�h��Ώ�(&N)";
      menu[IDM_EDIT_REMOVE_MARKED_EDGES_ON_CURRENT_LAYER_ALL]
        = L"���݂̃��C���[�̑I�����ꂽ�G�b�W��Ώ�(&E)";
      menu[IDM_EDIT_REMOVE_MARKED_ELEMENTS_ON_CURRENT_LAYER_ALL]
        = L"���݂̃��C���[�̑I�����ꂽ�m�[�h�ƃG�b�W��Ώ�(&A)";
      menu[IDM_EDIT_REMOVE_MARKED_NODES_ON_EACH_LAYER_ALL]
        = L"�S�Ẵ��C���[�̑I�����ꂽ�m�[�h��Ώ�(&N)";
      menu[IDM_EDIT_REMOVE_MARKED_EDGES_ON_EACH_LAYER_ALL]
        = L"�S�Ẵ��C���[�̑I�����ꂽ�G�b�W��Ώ�(&E)";
      menu[IDM_EDIT_REMOVE_MARKED_ELEMENTS_ON_EACH_LAYER_ALL]
        = L"�S�Ẵ��C���[�̑I�����ꂽ�m�[�h�ƃG�b�W��Ώ�(&A)";


      /////////////////////////////////////////////////////////////////////////////
      menu[IDM_LAYOUT]
        = L"���C�A�E�g(&L)";
      menu[IDM_LAYOUT_UPDATE]
        = L"�Čv�Z\tSpace";
      menu[IDM_LAYOUT_CANCEL]
        = L"�L�����Z��(&C)";
      menu[IDM_LAYOUT_KAMADA_KAWAI_METHOD]
        = L"Kamada-Kawai�@(&1)";
      menu[IDM_LAYOUT_HIGH_DIMENSIONAL_EMBEDDING]
        = L"High Dimensional Embedding(&2)";
      menu[IDM_LAYOUT_HIGH_DIMENSIONAL_EMBEDDING_1_2]
        = L"��1-2�ŗL����(&1)";
      menu[IDM_LAYOUT_HIGH_DIMENSIONAL_EMBEDDING_1_3]
        = L"��1-3�ŗL����(&2)";
      menu[IDM_LAYOUT_HIGH_DIMENSIONAL_EMBEDDING_2_3]
        = L"��2-3�ŗL����(&3)";
      menu[IDM_LAYOUT_CIRCLE]
        = L"�~�F�������(&3)";
      menu[IDM_LAYOUT_CIRCLE_IN_SIZE_ORDER]
        = L"�~�F�傫����(&4)";
      menu[IDM_LAYOUT_LATTICE]
        = L"�i�q(&5)";
      menu[IDM_LAYOUT_RANDOM]
        = L"�����_��(&6)";
      menu[IDM_LAYOUT_FORCE_DIRECTION]
        = L"���A���^�C���͊w�v�Z(&0)";
      menu[IDM_LAYOUT_FORCE_DIRECTION_RUN]
        = L"���s(&0)\tCtrl+Space";
      menu[IDM_LAYOUT_FORCE_DIRECTION_KAMADA_KAWAI_METHOD]
        = L"Kamada-Kawai�@(&1)";
      menu[IDM_LAYOUT_FORCE_DIRECTION_KAMADA_KAWAI_METHOD_WITH_COMMUNITY_SEPARATION]
        = L"Kamada-Kawai�@�{�R�~���j�e�B�Ԑ˗�(&2)";
      menu[IDM_LAYOUT_FORCE_DIRECTION_COMMUNITY_ORIENTED]
        = L"�R�~���j�e�B�w��(&3)";
      menu[IDM_LAYOUT_FORCE_DIRECTION_SPRING_AND_REPULSIVE_FORCE]
        = L"�΂ˁ{�˗�(&4)";
      menu[IDM_LAYOUT_FORCE_DIRECTION_LATTICE_GAS_METHOD]
        = L"�i�q�K�X�@(&5)";
      menu[IDM_LAYOUT_FORCE_DIRECTION_DESIGNTIDE]
        = L"DESIGNTIDE(&6)";
      menu[IDM_LAYOUT_SHOW_LAYOUT_FRAME]
        = L"���C�A�E�g�t���[���̕\��(&F)\tF";
      menu[IDM_LAYOUT_INITIALIZE_LAYOUT_FRAME]
        = L"���C�A�E�g�t���[���̏�����(&I)\tCtrl+Home";
      menu[IDM_LAYOUT_SHOW_GRID]
        = L"�O���b�h�̕\��(&G)\tG";
      menu[IDM_LAYOUT_SHOW_CENTER]
        = L"���S�_�ƒ����_�̕\��(&R)";
      menu[IDM_LAYOUT_INITIALIZE_EYEPOINT]
        = L"���_�̏�����(&I)\tHome";
      menu[IDM_LAYOUT_ZOOM_IN]
        = L"�g��(&Z)\tPage Down";
      menu[IDM_LAYOUT_ZOOM_OUT]
        = L"�k��(&X)\tPage Up";


      /////////////////////////////////////////////////////////////////////////////
      menu[IDM_COMMUNITY_DETECTION]
        = L"�R�~���j�e�B(&C)";
      menu[IDM_COMMUNITY_DETECTION_UPDATE]
        = L"�Čv�Z\tEnter";
      menu[IDM_COMMUNITY_DETECTION_CANCEL]
        = L"�L�����Z��(&C)";
      menu[IDM_COMMUNITY_TRANSITION_DIAGRAM]
        = L"�J�ڐ}�̕\��(&D)\tD";
      menu[IDM_COMMUNITY_TRANSITION_DIAGRAM_SCOPE_WIDER]
        = L"���L���G���A(&X)\tCtrl+��";
      menu[IDM_COMMUNITY_TRANSITION_DIAGRAM_SCOPE_NARROWER]
        = L"��苷���G���A(&Z)\tCtrl+��";
      menu[IDM_COMMUNITY_DETECTION_CONNECTED_COMPONENTS]
        = L"�A����������(&1)";
      menu[IDM_COMMUNITY_DETECTION_WEAKLY_CONNECTED_COMPONENTS]
        = L"��A����������(&1)";
      menu[IDM_COMMUNITY_DETECTION_STRONGLY_CONNECTED_COMPONENTS]
        = L"���A����������(&2)";
      menu[IDM_COMMUNITY_DETECTION_MODULARITY_MAXIMIZATION]
        = L"���W�������e�B�ő剻(&2)";
      menu[IDM_COMMUNITY_DETECTION_MODULARITY_MAXIMIZATION_USING_GREEDY_METHOD]
        = L"�~����@(&1)";
      menu[IDM_COMMUNITY_DETECTION_MODULARITY_MAXIMIZATION_USING_TEO_METHOD]
        = L"��-EO�@(&2)";
      menu[IDM_COMMUNITY_DETECTION_USE_WEIGHTED_MODULARITY]
        = L"�d�ݕt�����W�������e�B(&W)";
      menu[IDM_COMMUNITY_DETECTION_CLIQUE_PERCOLATION]
        = L"�N���[�N�p�[�R���[�V����(&3)";
      menu[IDM_COMMUNITY_DETECTION_CLIQUE_PERCOLATION_3]
        = L"3-�N���[�N�p�[�R���[�V����(&1)";
      menu[IDM_COMMUNITY_DETECTION_CLIQUE_PERCOLATION_4]
        = L"4-�N���[�N�p�[�R���[�V����(&2)";
      menu[IDM_COMMUNITY_DETECTION_CLIQUE_PERCOLATION_5]
        = L"5-�N���[�N�p�[�R���[�V����(&3)";
      menu[IDM_COMMUNITY_DETECTION_OTHERS]
        = L"���̑�(&4)";
      menu[IDM_COMMUNITY_DETECTION_BETWEENNESS_CENTRALITY_CLUSTERING]
        = L"�}��S������(&1)";
      menu[IDM_COMMUNITY_DETECTION_INFORMATION_FLOW_MAPPING]
        = L"Information Flow Mapping(&2)";
      menu[IDM_COMMUNITY_DETECTION_CLEAR]
        = L"������(&0)";


      /////////////////////////////////////////////////////////////////////////////
      menu[IDM_TIMELINE]
        = L"���n��(&T)";
      menu[IDM_TIMELINE_NEXT]
        = L"���̎���(&1)\t��";
      menu[IDM_TIMELINE_PREV]
        = L"�O�̎���(&2)\t��";
      menu[IDM_TIMELINE_AUTO_RUN]
        = L"����(&0)";
      menu[IDM_TIMELINE_STOP]
        = L"��~(&0)";
      menu[IDM_TIMELINE_FORWARD_1]
        = L"1�X�e�b�v/�b(&1)";
      menu[IDM_TIMELINE_FORWARD_2]
        = L"2�X�e�b�v/�b(&2)";
      menu[IDM_TIMELINE_FORWARD_3]
        = L"5�X�e�b�v/�b(&3)";
      menu[IDM_TIMELINE_FORWARD_4]
        = L"10�X�e�b�v/�b(&4)";
      menu[IDM_TIMELINE_BACKWARD_1]
        = L"-1�X�e�b�v/�b(&5)";
      menu[IDM_TIMELINE_BACKWARD_2]
        = L"-2�X�e�b�v/�b(&6)";
      menu[IDM_TIMELINE_BACKWARD_3]
        = L"-5�X�e�b�v/�b(&7)";
      menu[IDM_TIMELINE_BACKWARD_4]
        = L"-10�X�e�b�v/�b(&8)";
      menu[IDM_TIMELINE_SHOW_SLIDER]
        = L"���C���[�؂�ւ��X���C�_�[�̕\��(&S)\tS";
    }


  extern "C" __declspec(dllexport)
    void __cdecl load_message(vector<wstring>& message) {

      using namespace sociarium_project_language;

      assert(message.size()==Message::NUMBER_OF_MESSAGES);

      /////////////////////////////////////////////////////////////////////////////
      message[Message::QUIT]
        = L"�I�����܂���?";
      message[Message::IS_NOT_TEXTFILE]
        = L"�e�L�X�g�t�@�C���ł͂���܂���";


      /////////////////////////////////////////////////////////////////////////////
      message[Message::CALCULATING_CLOSENESS_CENTRALITY]
        = L"�������S�����v�Z���Ă��܂�";
      message[Message::CALCULATING_MEAN_SHORTEST_PATH_LENGTH]
        = L"���ύŒZ�o�H�����v�Z���Ă��܂�";
      message[Message::CALCULATING_CONNECTED_COMPONENTS]
        = L"��A���������v�Z���Ă��܂�";
      message[Message::CALCULATING_STRONGLY_CONNECTED_COMPONENTS]
        = L"���A���������v�Z���Ă��܂�";
      message[Message::CALCULATING_BETWEENNESS_CENTRALITY]
        = L"�}��S�����v�Z���Ă��܂�";
      message[Message::CALCULATING_PAGERANK]
        = L"�y�[�W�����N���v�Z���Ă��܂�";
      message[Message::CALCULATING_MODULARITY]
        = L"���W�������e�B���v�Z���Ă��܂�";
      message[Message::CALCULATING_LARGEST_CLIQUES]
        = L"�ő�N���[�N���v�Z���Ă��܂�";
      message[Message::CALCULATING_3_CLIQUE_COMMUNITIES]
        = L"3-�N���[�N�R�~���j�e�B���v�Z���Ă��܂�";
      message[Message::CALCULATING_DEGREE_CENTRALITY]
        = L"�������S�����v�Z���Ă��܂�";


      /////////////////////////////////////////////////////////////////////////////
      message[Message::GRAPH_EXTRACTOR_EXTRACTING_NODES]
        = L"�m�[�h�𒊏o���Ă��܂�";
      message[Message::GRAPH_EXTRACTOR_EXTRACTING_EDGES]
        = L"�G�b�W�𒊏o���Ă��܂�";


      /////////////////////////////////////////////////////////////////////////////
      message[Message::GLEW_FAILED_TO_INITIALIZE]
        = L"GLEW�̏������Ɏ��s���܂���";
      message[Message::GLEW_MSAA_8]
        = L"�}���`�T���v�����O�A���`�G���A�V���O(x8)���L���ɂȂ�܂���";
      message[Message::GLEW_MSAA_4]
        = L"�}���`�T���v�����O�A���`�G���A�V���O(x4)���L���ɂȂ�܂���";
      message[Message::GLEW_MSAA_2]
        = L"�}���`�T���v�����O�A���`�G���A�V���O(x2)���L���ɂȂ�܂���";
      message[Message::GLEW_FAILED_TO_ENABLE_MSAA]
        = L"�}���`�T���v�����O�A���`�G���A�V���O�����p�ł��܂���";


      /////////////////////////////////////////////////////////////////////////////
      message[Message::FAILED_TO_CREATE_TEXTURE]
        = L"�e�N�X�`���̐����Ɏ��s���܂���";


      /////////////////////////////////////////////////////////////////////////////
      message[Message::REMOVE_ELEMENTS]
        = L"�I�����ꂽ�v�f���폜���܂��B���ɂ͖߂��܂��񂪂�낵���ł���?";
      message[Message::CLEAR_COMMUNITY]
        =  L"�R�~���j�e�B�����폜���܂��B���ɂ͖߂��܂��񂪂�낵���ł���?";


      /////////////////////////////////////////////////////////////////////////////
      message[Message::GRAPH_IS_LOCKED]
        = L"�O���t���n��͑��̏����ɂ���ă��b�N����Ă��܂�";
      message[Message::ANOTHER_THREAD_IS_RUNNING]
        = L"�ʃX���b�h���N�����Ă��܂��B�I���܂ő҂��L�����Z����Ƀ��g���C���ĉ�����";
      message[Message::CANCEL_RUNNING_THREAD]
        = L"�N�����̃X���b�h���I�����܂���?";


      /////////////////////////////////////////////////////////////////////////////
      message[Message::READING_DATA_FILE]
        = L"�f�[�^��ǂݍ���ł��܂�";
      message[Message::CHECKING_TEXT_ENCODING]
        = L"�����R�[�h�𔻕ʂ��Ă��܂�";
      message[Message::CONVERTING_INTO_SJIS_ENCODING]
        = L"�����R�[�h��Shift_JIS�ɕϊ����Ă��܂�";
      message[Message::UNKNOWN_CHARACTER_ENCODING]
        = L"�s���ȃe�L�X�g�G���R�[�f�B���O�ł�";
      message[Message::PARSING_DATA]
        = L"�f�[�^����͂��Ă��܂�";
      message[Message::MAKING_GRAPH_TIME_SERIES]
        = L"�O���t���n����쐬���Ă��܂�";
      message[Message::MAKING_GRAPH_SNAPSHOT]
        = L"�O���t���쐬���Ă��܂�";
      message[Message::MAKING_GRAPH_ATTRIBUTES]
        = L"�O���t�v�f�̑������쐬���Ă��܂�";
      message[Message::MAKING_NODE_PROPERTIES]
        = L"�m�[�h�̑������쐬���Ă��܂�";
      message[Message::MAKING_EDGE_PROPERTIES]
        = L"�G�b�W�̑������쐬���Ă��܂�";
      message[Message::UNSUPPORTED_DATA_FORMAT]
        = L"�T�|�[�g����Ă��Ȃ��f�[�^�`���ł�";
      message[Message::FAILED_TO_READ_DATA]
        = L"�f�[�^�̓ǂݍ��݂Ɏ��s���܂���";
      message[Message::UNCERTAIN_DELIMITER]
        = L"�f�[�^�t�@�C���ɋ�؂蕶�����w�肳��Ă��܂���";
      message[Message::INVALID_NUMBER_OF_ITEMS]
        = L"�s�̍��ڐ�������������܂���";
      message[Message::NO_VALID_DATA]
        = L"�L���ȃf�[�^�����݂��܂���";
      message[Message::NODE_IDENTIFIER_DUPLICATION]
        = L"���ꃌ�C���[�Ɏ��ʎq�̏d������m�[�h�����݂��܂�";
      message[Message::EDGE_IDENTIFIER_DUPLICATION]
        = L"���ꃌ�C���[�Ɏ��ʎq�̏d������G�b�W�����݂��܂�";


      /////////////////////////////////////////////////////////////////////////////
      message[Message::KAMADA_KAWAI_METHOD]
        = L"Kamada-Kawai�@";
      message[Message::KAMADA_KAWAI_METHOD_CALCULATING_SPRING_STRENGTH]
        = L"�O���t��������o�l�W�����v�Z���Ă��܂�";
      message[Message::KAMADA_KAWAI_METHOD_ITERATING]
        = L"�C�e���[�V�������s��";
      message[Message::HDE]
        = L"High Dimensional Embedding";
      message[Message::HDE_CALCULATING_GRAPH_DISTANCE]
        = L"��_����̃O���t�������v�Z���Ă��܂�";
      message[Message::HDE_CALCULATING_MATRIX]
        = L"��_����̃O���t�����ɑ΂��镪�U�E�����U�s����v�Z���Ă��܂�";
      message[Message::HDE_CALCULATING_PRINCIPAL_COMPONENTS]
        = L"���U�E�����U�s��̎听�����v�Z���Ă��܂�";
      message[Message::HDE_CALCULATING_POSITION]
        = L"���U�E�����U�s��̎听��������W���v�Z���Ă��܂�";


      /////////////////////////////////////////////////////////////////////////////
      message[Message::DETECTING_COMMUNITIES]
        = L"�R�~���j�e�B���o���s���Ă��܂�";
      message[Message::MAKING_COMMUNITY_TIME_SERIES]
        = L"�R�~���j�e�B���n����쐬���Ă��܂�";
      message[Message::MODULARITY_MAXIMIZATION_USING_GREEDY_METHOD]
        = L"���W�������e�B�ő剻 [�~����@]";
      message[Message::MODULARITY_MAXIMIZATION_USING_TEO_METHOD]
        = L"���W�������e�B�ő剻 [��-EO�@]";
      message[Message::BETWEENNESS_CENTRALITY_CLUSTERING]
        = L"�}��S���N���X�^�����O";
      message[Message::INFORMATION_FLOW_MAPPING]
        = L"Information Flow Mapping";


      /////////////////////////////////////////////////////////////////////////////
      message[Message::UPDATING_NODE_SIZE]
        = L"�m�[�h�̑傫�����X�V���Ă��܂�";
      message[Message::UPDATING_EDGE_WIDTH]
        = L"�G�b�W�̑������X�V���Ă��܂�";


      ////////////////////////////////////////////////////////////////////////////////
      message[Message::FTGL_ERROR_CHARMAP]
        = L"FTFont::CharMap�̌Ăяo���Ɏ��s���܂���";
      message[Message::FTGL_ERROR_FACESIZE]
        = L"FTFont::FaceSize�̌Ăяo���Ɏ��s���܂���";
      message[Message::FTGL_ERROR_CREATE]
        = L"Failed to create a font object";
    }

} // The end of the namespace "hashimoto_ut"
