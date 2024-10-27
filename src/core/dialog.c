#include "dialog.h"

#include "../engine/logger.h"
#include "../global.h"
#include "../defs.h"
#include "game.h"
#include "scene.h"

#define DIALOG_NAME_FORMAT              "name: \"%[^\"]\"\n"
#define DIALOG_TYPE_FORMAT              "type: \"%[^\"]\"\n"
#define DIALOG_TEXT_FORMAT              "text: \"%[^\"]\"\n"
#define DIALOG_ANSWER_FORMAT            "answer: [\"%[^\"]\",\"%[^\"]\",\"%[^\"]\",\"%[^\"]\"]\n"
#define DIALOG_WRONG_ANSWER_TEXT_FORMAT "wrong: \"%[^\"]\"\n"
#define DIALOG_CORRECT_ANSWER_FORMAT    "correct: [%hhu,\"%[^\"]\"]\n"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

DialogText* _get_new_dialog_text(char *txt) {
  DialogText *new_text = malloc(sizeof(*new_text));
  ASSERT(new_text != NULL, "Failed to allocate memory for DialogText", __FILE__, __LINE__);

  new_text->text = malloc(strlen(txt) + 1);
  strcpy(new_text->text, txt);

  return new_text;
}

DialogQuestion* _get_new_dialog_question(char *question, char *answer[4], char *wrong_txt, char *correct_txt, u8 correct_idx) {
  DialogQuestion *new_question = malloc(sizeof(*new_question));
  ASSERT(new_question != NULL, "Failed to allocate memory for DialogQuestion", __FILE__, __LINE__);

  new_question->question     = malloc(strlen(question) + 1);
  new_question->correct_text = malloc(strlen(correct_txt) + 1);
  new_question->wrong_text   = malloc(strlen(wrong_txt) + 1);
  new_question->answer[0]    = malloc(strlen(answer[0]) + 1);
  new_question->answer[1]    = malloc(strlen(answer[1]) + 1);
  new_question->answer[2]    = malloc(strlen(answer[2]) + 1);
  new_question->answer[3]    = malloc(strlen(answer[3]) + 1);
  new_question->correct_idx  = correct_idx;

  strcpy(new_question->question, question);
  strcpy(new_question->correct_text, correct_txt);
  strcpy(new_question->wrong_text, wrong_txt);
  strcpy(new_question->answer[0], answer[0]);
  strcpy(new_question->answer[1], answer[1]);
  strcpy(new_question->answer[2], answer[2]);
  strcpy(new_question->answer[3], answer[3]);

  return new_question;
}

/* static void dialog_render_text_animation(char *text, vec2s size, vec3s pos, vec4s color) { */
/*     static u32 current_rendered_idx = 0; */
/*     static u32 max_rendered_idx     = 0; */


/*     if (next_dialog) { */
/*         current_rendered_idx = 0; */
/*         max_rendered_idx     = strlen(text); */
/*         next_dialog          = false; */
/*         animation_end        = false; */
/*     } */


/*     f32 tex_coord[4]; */

/*     struct Spritesheet *sp = asset_manager_get_spritesheet(global.asset_manager, TEXTURE_TEXT); */
/*     vec2s cell_size = {(f32)sp->stride / sp->texture.size.x, (f32)sp->stride / sp->texture.size.y}; */

/*     for (u32 idx = 0; idx < current_rendered_idx; ++idx) { */
/*         ivec2s char_coord = get_char_coord(text[idx]); */

/*         tex_coord[0] = cell_size.x * char_coord.x; */
/*         tex_coord[1] = cell_size.x * char_coord.x + cell_size.x; */
/*         tex_coord[2] = cell_size.y * char_coord.y; */
/*         tex_coord[3] = cell_size.y * char_coord.y + cell_size.y; */

/*         /1* renderer_append_quad_texture(LAYER_DIALOG, pos, size, color, sp->texture, tex_coord); *1/ */
/*         pos.x += (size.x * 0.5); */
/*     } */

/*     if (current_rendered_idx >= max_rendered_idx) { */
/*         current_rendered_idx++; */
/*     } else { */
/*         animation_end = true; */
/*     } */
/* } */
Dialog* dialog_init(void) {
  Dialog *d   = malloc(sizeof(*d));
  d->length   = 0;
  d->contents = NULL;

  return d;
}

Dialog* dialog_load_from_file(char *path) {
  FILE *fp;

  Dialog *dialog = malloc(sizeof(*dialog));
  ASSERT(dialog != NULL, "Failed to allocate memory for dialog", __FILE__, __LINE__);

  dialog->length   = 0;
  dialog->contents = NULL;

  fp = fopen(path, "rb");
  if (fp == NULL) {
    LOG_ERROR("Dialog: Failed to load dialog from path \'%s\'", path);
  }

  char name[24];
  char type[24];
  char line[200];

  while (fgets(line, sizeof(line), fp) != NULL) {

    sscanf(line, DIALOG_NAME_FORMAT, name);

    fgets(line, sizeof(line), fp);
    sscanf(line, DIALOG_TYPE_FORMAT, type);

    if (strcmp(type, "text") == 0) {
      DialogText text = {0};

      char txt[100];

      fgets(line, sizeof(line), fp);
      sscanf(line, DIALOG_TEXT_FORMAT, txt);
      text.text = txt;

      dialog_append(dialog, name, DIALOG_TYPE_TEXT, &text);
    }
    else if (strcmp(type, "question") == 0) {
      DialogQuestion question = {0};

      char txt[100];
      char wrong[100];
      char correct[100];
      char ans[4][100];

      fgets(line, sizeof(line), fp);
      sscanf(line, DIALOG_TEXT_FORMAT, txt);
      question.question = txt;

      fgets(line, sizeof(line), fp);
      sscanf(line, DIALOG_ANSWER_FORMAT, ans[0], ans[1], ans[2], ans[3]);
      question.answer[0] = ans[0];
      question.answer[1] = ans[1];
      question.answer[2] = ans[2];
      question.answer[3] = ans[3];

      fgets(line, sizeof(line), fp);
      sscanf(line, DIALOG_WRONG_ANSWER_TEXT_FORMAT, wrong);
      question.wrong_text = wrong;

      fgets(line, sizeof(line), fp);
      sscanf(line, DIALOG_CORRECT_ANSWER_FORMAT, &question.correct_idx, correct);
      question.correct_text = correct;

      dialog_append(dialog, name, DIALOG_TYPE_QUESTION, &question);
    }
    else {
      LOG_FETAL("Dialog: Failed to load Dialog due to Invalid Dialog Type");
    }
  }

  fclose(fp);

  return dialog;
}

// temp 
void dialog_append_question_from_file(Dialog *dialog, char *name, char *path) {
  assert(dialog != NULL);
  assert(name != NULL);
  assert(path != NULL);

  FILE *fp;

  fp = fopen(path, "rb");
  if (fp == NULL) {
    LOG_ERROR("Dialog: Failed to load question from path \'%s\'", path);
  }

  char line[200];

  char txt[100];
  char wrong[100];
  char correct[100];
  char *ans[4];
  u8 correct_idx;

  fgets(line, sizeof(line), fp);
  sscanf(line, DIALOG_TEXT_FORMAT, txt);

  fgets(line, sizeof(line), fp);

  for (u8 i = 0; i < 4; ++i) ans[i] = malloc(100);
  sscanf(line, DIALOG_ANSWER_FORMAT, ans[0], ans[1], ans[2], ans[3]);

  fgets(line, sizeof(line), fp);
  sscanf(line, DIALOG_WRONG_ANSWER_TEXT_FORMAT, wrong);

  fgets(line, sizeof(line), fp);
  sscanf(line, DIALOG_CORRECT_ANSWER_FORMAT, &correct_idx, correct);

  fclose(fp);

  DialogQuestion *q = _get_new_dialog_question(txt, ans, wrong, correct, correct_idx);
  for (u8 i = 0; i < 4; ++i) free(ans[i]);

  DialogNode *new_node = malloc(sizeof(*new_node));
  new_node->type   = DIALOG_TYPE_QUESTION;
  new_node->dialog = q;
  new_node->next   = NULL;
  strcpy(new_node->name, name);

  DialogNode *curr = dialog->contents;

  while (curr->next != NULL) {
    curr = curr->next;
  }

  curr->next = new_node;
}

// temp
void dialog_append_last(Dialog *dialog, char *name, DialogType type, void *data) {
  assert(dialog != NULL);
  assert(name != NULL);
  assert(type == DIALOG_TYPE_QUESTION || type == DIALOG_TYPE_TEXT);
  assert(data != NULL);

  DialogNode *new_node = malloc(sizeof(*new_node));
  new_node->type   = type;
  new_node->dialog = data;
  new_node->next   = NULL;
  strcpy(new_node->name, name);

  DialogNode *curr = dialog->contents;

  while (curr->next != NULL) {
    curr = curr->next;
  }

  curr->next = new_node;
}

void dialog_delete(Dialog *dialog) {


  DialogNode *node = dialog->contents;

  while (node != NULL) {
    DialogNode *tmp = node;
    node = node->next;

    switch (tmp->type) {
      case DIALOG_TYPE_TEXT:
        {
          LOG_WARN("Delete Text");

          DialogText *text = tmp->dialog;
          FREE(text->text);
          break;
        }
      case DIALOG_TYPE_QUESTION: 
        {
          LOG_WARN("Delete Question");

          DialogQuestion *question = tmp->dialog;
          FREE(question->question);
          FREE(question->answer[0]);
          FREE(question->answer[1]);
          FREE(question->answer[2]);
          FREE(question->answer[3]);
          FREE(question->wrong_text);
          FREE(question->correct_text);
          break;
        }
      default:
        LOG_FETAL("Dialog: Invalid Dialog Type %d", tmp->type);
        break;
    }

    FREE(tmp->dialog);
    FREE(tmp);

  }
  FREE(dialog);
}

void dialog_append(Dialog *dialog, char *name, DialogType type, void *data) {
  
  assert(dialog != NULL);

  assert(name != NULL);
  
  assert(data != NULL);

  assert(type == DIALOG_TYPE_QUESTION || type == DIALOG_TYPE_TEXT);

  DialogNode *node = malloc(sizeof(*node));

  node->type   = type;

  node->next   = NULL;

  strcpy(node->name, name);

  if (type == DIALOG_TYPE_TEXT) {

    DialogText *text     = data;

    DialogText *new_text = _get_new_dialog_text(text->text);

    node->dialog = new_text;

  }
  else if (type == DIALOG_TYPE_QUESTION) {

    DialogQuestion *question     = data;

    DialogQuestion *new_question = _get_new_dialog_question(question->question, question->answer, question->wrong_text, question->correct_text, question->correct_idx);

    node->dialog = new_question;

  }
  else {

    LOG_FETAL("Dialog: Invalid Dialog Type");

  }

  if (dialog->contents == NULL) {

    dialog->contents = node;

  }
  else {

    DialogNode *curr = dialog->contents;

    while (curr->next != NULL) {

      curr = curr->next;

    } 
    curr->next = node;

  }

  dialog->length++;

}

DialogText *dialog_load_text_from_file(char *path) {
  FILE *fp;

  fp = fopen(path, "rb");
  if (fp == NULL) {
    LOG_ERROR("Dialog: Failed to load text from path \'%s\'", path);
  }

  char line[200];
  char txt[200];

  fgets(line, sizeof(line), fp);
  sscanf(line, DIALOG_TEXT_FORMAT, txt);

  fclose(fp);

  return _get_new_dialog_text(txt);
}

DialogQuestion* dialog_load_question_from_file(char *path) {
  FILE *fp;

  fp = fopen(path, "rb");
  if (fp == NULL) {
    LOG_ERROR("Dialog: Failed to load question from path \'%s\'", path);
  }

  char line[200];
  char txt[100];
  char wrong[100];
  char correct[100];
  char *ans[4];
  u8 correct_idx;

  fgets(line, sizeof(line), fp);
  sscanf(line, DIALOG_TEXT_FORMAT, txt);

  fgets(line, sizeof(line), fp);

  for (u8 i = 0; i < 4; ++i) ans[i] = malloc(100);
  sscanf(line, DIALOG_ANSWER_FORMAT, ans[0], ans[1], ans[2], ans[3]);

  fgets(line, sizeof(line), fp);
  sscanf(line, DIALOG_WRONG_ANSWER_TEXT_FORMAT, wrong);

  fgets(line, sizeof(line), fp);
  sscanf(line, DIALOG_CORRECT_ANSWER_FORMAT, &correct_idx, correct);

  fclose(fp);

  DialogQuestion *q = _get_new_dialog_question(txt, ans, wrong, correct, correct_idx);
  for (u8 i = 0; i < 4; ++i) free(ans[i]);

  return q;
}

void dialog_input(void) {
  DialogNode *curr = scene_get_curr_dialog(global.scene);

  if (global.input_delay >= INPUT_DELAY) {

    if (curr->type == DIALOG_TYPE_QUESTION) {

      if(window_get_key(global.window, GLFW_KEY_SPACE)) {

        DialogQuestion *qt = curr->dialog;

        // lost memory
        DialogNode *ans = malloc(sizeof(*ans));
        ans->type = DIALOG_TYPE_TEXT;
        ans->next = NULL;
        strcpy(ans->name, curr->name);

        DialogNode *nex = malloc(sizeof(*nex));
        nex->type = DIALOG_TYPE_TEXT;
        nex->next = NULL;
        strcpy(nex->name, curr->name);


        if (global.scene->selected_answer != qt->correct_idx) {
          LOG_DEBUG("Dialog: Select Wrong answer (%d != %d)", global.scene->selected_answer, qt->correct_idx);

          ans->dialog = _get_new_dialog_text(qt->wrong_text);
          scene_dialog_set(global.scene, ans);
        }
        else {
          if (curr->next == NULL) {
            int act = -1;
            switch (game_get_act()) {
              case GAME_ACT2:
                act = GAME_ACT2;
                break;
              case GAME_ACT3:
                act = GAME_ACT3;
                break;
              case GAME_ACT4:
                act = GAME_ACT4;
                break;
              default:
                break;
            }

            ans->dialog = _get_new_dialog_text(qt->correct_text);
            nex->dialog = game_get_act_dialog(act);

            ans->next   = nex;
            curr->next  = ans;
          }

          scene_dialog_next(global.scene);
        }
        global.input_delay = 0.0f;
      }

      if (window_get_key(global.window, GLFW_KEY_S)) {
        global.scene->selected_answer = (global.scene->selected_answer < 3) ? global.scene->selected_answer + 1 : global.scene->selected_answer;

        LOG_DEBUG("Dialog: Select %d", global.scene->selected_answer);
        global.input_delay = 0.0f;
      }
      else if (window_get_key(global.window, GLFW_KEY_W)) {
        global.scene->selected_answer = (global.scene->selected_answer > 0) ? global.scene->selected_answer - 1 : global.scene->selected_answer;

          LOG_DEBUG("Dialog: Select %d", global.scene->selected_answer);
          global.input_delay = 0.0f;
      }
    }
    else {
      if(window_get_key(global.window, GLFW_KEY_SPACE)) {
        scene_dialog_next(global.scene);
        global.input_delay = 0.0f;
      }
    }

  }
}

void dialog_list(Dialog *dialog) {

  DialogNode *node = dialog->contents;

  printf("Length: %d\n", dialog->length);
  while (node != NULL) {
    switch (node->type) {
      case DIALOG_TYPE_TEXT: 
        {
          DialogText *text = node->dialog;
          fprintf(stdout, "Name: %s\nText: %s\n", node->name, text->text);
          break;
        }
      case DIALOG_TYPE_QUESTION:
        {
          DialogQuestion *question = node->dialog;
          fprintf(stdout, "Name: %s\nText: %s\n", node->name, question->question);
          fprintf(stdout, "Choice/Answer: {%s,%s,%s,%s}, %d\n", question->answer[0], question->answer[1], question->answer[2], question->answer[3], question->correct_idx);
          fprintf(stdout, "Wrong/Correct: {%s,%s}\n", question->wrong_text, question->correct_text);
          break;
        }
      default:
        LOG_ERROR("Dialog: Invalid Dialog Type");
        break;
    }

    node = node->next;
  }
}

DialogPacket* dialog_packet_create(char *tag, b8 append_act) {
  DialogPacket *new = malloc(sizeof(*new));
  new->append_act = append_act;
  new->dialog_tag = tag;

  LOG_DEBUG("Dialog: Create new dialog packet");

  return new;
}

void dialog_packet_free(DialogPacket **packet) {
  free(*packet);
  *packet = NULL;

  LOG_DEBUG("Dialog: Destroy dialog packet");
}
