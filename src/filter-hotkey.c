/*
 * OBS Filter Hotkeys
 * Copyright (C) 2026 Fabio Ventura
 *
 * Controlador que associa uma hotkey independente a um filtro-alvo
 * dentro da mesma fonte do OBS.
 */

#include <obs-module.h>
#include <obs-hotkey.h>
#include <obs-interaction.h>
#include <util/bmem.h>

#ifdef _WIN32
#include <windows.h>
typedef CRITICAL_SECTION filter_hotkey_mutex_t;
static void filter_hotkey_mutex_init(filter_hotkey_mutex_t *mutex) { InitializeCriticalSection(mutex); }
static void filter_hotkey_mutex_destroy(filter_hotkey_mutex_t *mutex) { DeleteCriticalSection(mutex); }
static void filter_hotkey_mutex_lock(filter_hotkey_mutex_t *mutex) { EnterCriticalSection(mutex); }
static void filter_hotkey_mutex_unlock(filter_hotkey_mutex_t *mutex) { LeaveCriticalSection(mutex); }
#else
#include <pthread.h>
typedef pthread_mutex_t filter_hotkey_mutex_t;
static void filter_hotkey_mutex_init(filter_hotkey_mutex_t *mutex) { pthread_mutex_init(mutex, NULL); }
static void filter_hotkey_mutex_destroy(filter_hotkey_mutex_t *mutex) { pthread_mutex_destroy(mutex); }
static void filter_hotkey_mutex_lock(filter_hotkey_mutex_t *mutex) { pthread_mutex_lock(mutex); }
static void filter_hotkey_mutex_unlock(filter_hotkey_mutex_t *mutex) { pthread_mutex_unlock(mutex); }
#endif

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define SETTING_TARGET_FILTER "target_filter"
#define SETTING_MODE "mode"
#define SETTING_HOTKEY "hotkey"

#define MODE_TOGGLE 0
#define MODE_ENABLE 1
#define MODE_DISABLE 2

struct filter_hotkey_data {
	obs_source_t *context;
	obs_source_t *parent;
	obs_hotkey_id hotkey_id;
	char *target_filter;
	char *hotkey_text;
	int mode;
	filter_hotkey_mutex_t mutex;
};

static const char *filter_hotkey_get_name(void *unused)
{
	UNUSED_PARAMETER(unused);
	return "Filter Hotkeys - Controlador de filtros";
}

static const char *filter_hotkey_mode_label(int mode)
{
	switch (mode) {
	case MODE_ENABLE:
		return "enable";
	case MODE_DISABLE:
		return "disable";
	default:
		return "toggle";
	}
}

static void filter_hotkey_trim(char *text)
{
	size_t length;
	char *start;

	if (!text)
		return;

	length = strlen(text);
	while (length > 0 && isspace((unsigned char)text[length - 1]))
		text[--length] = '\0';

	start = text;
	while (*start && isspace((unsigned char)*start))
		start++;
	if (start != text)
		memmove(text, start, strlen(start) + 1);
}

static void filter_hotkey_uppercase(char *text)
{
	if (!text)
		return;
	for (; *text; ++text)
		*text = (char)toupper((unsigned char)*text);
}

static obs_key_t filter_hotkey_key_from_user_name(const char *token)
{
	char key_name[128];

	if (!token || !token[0])
		return OBS_KEY_NONE;

	if (strncmp(token, "OBS_KEY_", 8) == 0)
		return obs_key_from_name(token);

	if (snprintf(key_name, sizeof(key_name), "OBS_KEY_%s", token) < 0)
		return OBS_KEY_NONE;

	return obs_key_from_name(key_name);
}

static bool filter_hotkey_parse_binding(const char *text, obs_key_combination_t *combination)
{
	char *copy;
	char *token;
	bool valid = true;
	bool have_key = false;

	if (!text || !text[0] || !combination)
		return false;

	combination->modifiers = 0;
	combination->key = OBS_KEY_NONE;
	copy = bstrdup(text);
	token = strtok(copy, "+");

	while (token && valid) {
		filter_hotkey_trim(token);
		filter_hotkey_uppercase(token);

		if (!token[0]) {
			valid = false;
		} else if (strcmp(token, "CTRL") == 0 || strcmp(token, "CONTROL") == 0) {
			combination->modifiers |= INTERACT_CONTROL_KEY;
		} else if (strcmp(token, "SHIFT") == 0) {
			combination->modifiers |= INTERACT_SHIFT_KEY;
		} else if (strcmp(token, "ALT") == 0) {
			combination->modifiers |= INTERACT_ALT_KEY;
		} else if (strcmp(token, "WIN") == 0 || strcmp(token, "META") == 0 || strcmp(token, "CMD") == 0) {
			combination->modifiers |= INTERACT_COMMAND_KEY;
			} else if (have_key) {
				valid = false;
			} else {
				combination->key = filter_hotkey_key_from_user_name(token);
				have_key = combination->key != OBS_KEY_NONE;
				if (!have_key)
					valid = false;
			}

		token = strtok(NULL, "+");
	}

	bfree(copy);
	return valid && have_key;
}

struct filter_hotkey_binding_update {
	obs_hotkey_id id;
	obs_key_combination_t combination;
};

static void filter_hotkey_load_binding_atomic(void *ptr)
{
	struct filter_hotkey_binding_update *update = ptr;
	obs_hotkey_load_bindings(update->id, &update->combination, 1);
}

static void filter_hotkey_apply_binding(obs_hotkey_id id, const char *text)
{
	obs_key_combination_t combination;
	struct filter_hotkey_binding_update update;

	if (id == OBS_INVALID_HOTKEY_ID || !text || !text[0])
		return;

	if (!filter_hotkey_parse_binding(text, &combination)) {
		blog(LOG_WARNING, "Filter Hotkeys: invalid hotkey '%s'. Use formats like F8 or Ctrl+Shift+F8", text);
		return;
	}

	update.id = id;
	update.combination = combination;
	obs_hotkey_update_atomic(filter_hotkey_load_binding_atomic, &update);
	blog(LOG_INFO, "Filter Hotkeys: binding aplicada '%s' -> %s (modifiers=%u)", text,
	     obs_key_to_name(combination.key), (unsigned int)combination.modifiers);
}

static void filter_hotkey_update_description(struct filter_hotkey_data *data)
{
	char *target;
	char description[512];
	obs_hotkey_id hotkey_id;

	filter_hotkey_mutex_lock(&data->mutex);
	target = bstrdup(data->target_filter ? data->target_filter : "");
	hotkey_id = data->hotkey_id;
	filter_hotkey_mutex_unlock(&data->mutex);

	if (hotkey_id != OBS_INVALID_HOTKEY_ID) {
		if (target[0])
			snprintf(description, sizeof(description), "Filter Hotkeys: %s", target);
		else
			snprintf(description, sizeof(description), "Filter Hotkeys: escolha um filtro");
		obs_hotkey_set_description(hotkey_id, description);
	}

	bfree(target);
}

static const char *filter_hotkey_get_target(struct filter_hotkey_data *data)
{
	return data->target_filter ? data->target_filter : "";
}

static bool filter_hotkey_target_modified(void *priv, obs_properties_t *props, obs_property_t *property,
						  obs_data_t *settings)
{
	struct filter_hotkey_data *data = priv;
	const char *text;
	char *copy;

	UNUSED_PARAMETER(props);
	UNUSED_PARAMETER(property);

	if (!data || !settings)
		return false;

	text = obs_data_get_string(settings, SETTING_TARGET_FILTER);
	copy = bstrdup(text ? text : "");
	filter_hotkey_mutex_lock(&data->mutex);
	bfree(data->target_filter);
	data->target_filter = copy;
	filter_hotkey_mutex_unlock(&data->mutex);
	filter_hotkey_update_description(data);
	return false;
}

static bool filter_hotkey_mode_modified(void *priv, obs_properties_t *props, obs_property_t *property,
						obs_data_t *settings)
{
	struct filter_hotkey_data *data = priv;
	int mode;

	UNUSED_PARAMETER(props);
	UNUSED_PARAMETER(property);

	if (!data || !settings)
		return false;

	mode = (int)obs_data_get_int(settings, SETTING_MODE);
	filter_hotkey_mutex_lock(&data->mutex);
	data->mode = (mode >= MODE_TOGGLE && mode <= MODE_DISABLE) ? mode : MODE_TOGGLE;
	filter_hotkey_mutex_unlock(&data->mutex);
	return false;
}

static bool filter_hotkey_hotkey_modified(void *priv, obs_properties_t *props, obs_property_t *property,
						 obs_data_t *settings)
{
	struct filter_hotkey_data *data = priv;
	const char *text;
	char *copy;

	UNUSED_PARAMETER(props);
	UNUSED_PARAMETER(property);

	if (!data || !settings)
		return false;

	text = obs_data_get_string(settings, SETTING_HOTKEY);
	copy = bstrdup(text ? text : "");
	filter_hotkey_mutex_lock(&data->mutex);
	bfree(data->hotkey_text);
	data->hotkey_text = copy;
	filter_hotkey_mutex_unlock(&data->mutex);
	return false;
}

static bool filter_hotkey_save_clicked(obs_properties_t *props, obs_property_t *property, void *priv)
{
	struct filter_hotkey_data *data = priv;
	char *text;
	obs_hotkey_id hotkey_id;
	obs_key_combination_t combination;

	UNUSED_PARAMETER(props);
	UNUSED_PARAMETER(property);

	if (!data)
		return false;

	filter_hotkey_mutex_lock(&data->mutex);
	text = bstrdup(data->hotkey_text ? data->hotkey_text : "");
	hotkey_id = data->hotkey_id;
	filter_hotkey_mutex_unlock(&data->mutex);

	if (!filter_hotkey_parse_binding(text, &combination)) {
		blog(LOG_WARNING, "Filter Hotkeys: bind invalida '%s'. Use F8, Ctrl+F8 ou Ctrl+Shift+F8", text);
		bfree(text);
		return false;
	}

	filter_hotkey_apply_binding(hotkey_id, text);
	blog(LOG_INFO, "Filter Hotkeys: bind salva '%s'", text);
	bfree(text);
	return false;
}

static void filter_hotkey_update(void *data_ptr, obs_data_t *settings)
{
	struct filter_hotkey_data *data = data_ptr;
	const char *target = obs_data_get_string(settings, SETTING_TARGET_FILTER);
	const char *hotkey = obs_data_get_string(settings, SETTING_HOTKEY);
	char *new_target = bstrdup(target ? target : "");
	char *new_hotkey = bstrdup(hotkey ? hotkey : "");
	char *old_target;
	char *old_hotkey;
	obs_hotkey_id hotkey_id;
	int new_mode = (int)obs_data_get_int(settings, SETTING_MODE);

	filter_hotkey_mutex_lock(&data->mutex);
	old_target = data->target_filter;
	old_hotkey = data->hotkey_text;
	data->target_filter = new_target;
	data->hotkey_text = new_hotkey;
	data->mode = (new_mode >= MODE_TOGGLE && new_mode <= MODE_DISABLE) ? new_mode : MODE_TOGGLE;
	hotkey_id = data->hotkey_id;
	filter_hotkey_mutex_unlock(&data->mutex);

	bfree(old_target);
	bfree(old_hotkey);

	filter_hotkey_apply_binding(hotkey_id, new_hotkey);
	filter_hotkey_update_description(data);
}

static void filter_hotkey_set_parent(struct filter_hotkey_data *data, obs_source_t *parent)
{
	obs_source_t *new_parent = parent ? obs_source_get_ref(parent) : NULL;
	obs_source_t *old_parent;

	filter_hotkey_mutex_lock(&data->mutex);
	old_parent = data->parent;
	data->parent = new_parent;
	filter_hotkey_mutex_unlock(&data->mutex);

	if (old_parent)
		obs_source_release(old_parent);
}

static void filter_hotkey_callback(void *data_ptr, obs_hotkey_id id, obs_hotkey_t *hotkey, bool pressed)
{
	struct filter_hotkey_data *data = data_ptr;
	obs_source_t *parent;
	obs_source_t *target;
	char *target_filter;
	int mode;
	bool next_state;

	UNUSED_PARAMETER(id);
	UNUSED_PARAMETER(hotkey);

	if (!pressed)
		return;

	blog(LOG_INFO, "Filter Hotkeys: callback disparado");
	filter_hotkey_mutex_lock(&data->mutex);
	parent = data->parent ? obs_source_get_ref(data->parent) : NULL;
	target_filter = bstrdup(data->target_filter ? data->target_filter : "");
	mode = data->mode;
	filter_hotkey_mutex_unlock(&data->mutex);

	if (!parent || !target_filter[0]) {
		if (parent)
			obs_source_release(parent);
		bfree(target_filter);
		blog(LOG_WARNING, "Filter Hotkeys: escolha um filtro-alvo nas propriedades do controlador");
		return;
	}

	target = obs_source_get_filter_by_name(parent, target_filter);
	if (!target) {
		blog(LOG_WARNING, "Filter Hotkeys: filter '%s' was not found on source '%s'", target_filter,
		     obs_source_get_name(parent));
		obs_source_release(parent);
		bfree(target_filter);
		return;
	}

	switch (mode) {
	case MODE_ENABLE:
		next_state = true;
		break;
	case MODE_DISABLE:
		next_state = false;
		break;
	default:
		next_state = !obs_source_enabled(target);
		break;
	}

	obs_source_set_enabled(target, next_state);
	blog(LOG_INFO, "Filter Hotkeys: %s '%s' on source '%s' (%s)", next_state ? "enabled" : "disabled",
	     target_filter, obs_source_get_name(parent), filter_hotkey_mode_label(mode));
	obs_source_release(target);
	obs_source_release(parent);
	bfree(target_filter);
}

static bool filter_hotkey_test_clicked(obs_properties_t *props, obs_property_t *property, void *priv)
{
	UNUSED_PARAMETER(props);
	UNUSED_PARAMETER(property);
	filter_hotkey_callback(priv, OBS_INVALID_HOTKEY_ID, NULL, true);
	return false;
}

static struct obs_source_frame *filter_hotkey_video(void *data_ptr, struct obs_source_frame *frame)
{
	struct filter_hotkey_data *data = data_ptr;

	if (!data->parent)
		filter_hotkey_set_parent(data, obs_filter_get_parent(data->context));
	return frame;
}

static void filter_hotkey_render(void *data_ptr, gs_effect_t *effect)
{
	struct filter_hotkey_data *data = data_ptr;

	UNUSED_PARAMETER(effect);
	if (!data->parent)
		filter_hotkey_set_parent(data, obs_filter_get_parent(data->context));
	obs_source_skip_video_filter(data->context);
}

static void filter_hotkey_add(void *data_ptr, obs_source_t *parent)
{
	struct filter_hotkey_data *data = data_ptr;
	filter_hotkey_set_parent(data, parent);
}

static void filter_hotkey_remove(void *data_ptr, obs_source_t *parent)
{
	struct filter_hotkey_data *data = data_ptr;

	UNUSED_PARAMETER(parent);
	filter_hotkey_set_parent(data, NULL);
}

static void *filter_hotkey_create(obs_data_t *settings, obs_source_t *source)
{
	struct filter_hotkey_data *data = bzalloc(sizeof(*data));
	char *initial_hotkey;

	data->context = source;
	data->hotkey_id = OBS_INVALID_HOTKEY_ID;
	filter_hotkey_mutex_init(&data->mutex);
	filter_hotkey_update(data, settings);

	char hotkey_name[160];
	const char *uuid = obs_source_get_uuid(source);
	if (uuid && uuid[0])
		snprintf(hotkey_name, sizeof(hotkey_name), "filter_hotkeys_action_%s", uuid);
	else
		snprintf(hotkey_name, sizeof(hotkey_name), "filter_hotkeys_action_%p", (void *)source);

	data->hotkey_id = obs_hotkey_register_frontend(hotkey_name, "Atalho do Filter Hotkeys",
							      filter_hotkey_callback, data);
	if (data->hotkey_id == OBS_INVALID_HOTKEY_ID) {
		blog(LOG_ERROR, "Filter Hotkeys: could not register source hotkey");
	} else {
		filter_hotkey_mutex_lock(&data->mutex);
		initial_hotkey = bstrdup(data->hotkey_text ? data->hotkey_text : "");
		filter_hotkey_mutex_unlock(&data->mutex);
		filter_hotkey_apply_binding(data->hotkey_id, initial_hotkey);
		bfree(initial_hotkey);
		filter_hotkey_update_description(data);
	}

	return data;
}

static void filter_hotkey_destroy(void *data_ptr)
{
	struct filter_hotkey_data *data = data_ptr;

	if (data->hotkey_id != OBS_INVALID_HOTKEY_ID)
		obs_hotkey_unregister(data->hotkey_id);
	filter_hotkey_set_parent(data, NULL);
	filter_hotkey_mutex_lock(&data->mutex);
	bfree(data->target_filter);
	bfree(data->hotkey_text);
	data->target_filter = NULL;
	data->hotkey_text = NULL;
	filter_hotkey_mutex_unlock(&data->mutex);
	filter_hotkey_mutex_destroy(&data->mutex);
	bfree(data);
}

static void filter_hotkey_defaults(obs_data_t *settings)
{
	obs_data_set_default_string(settings, SETTING_TARGET_FILTER, "");
	obs_data_set_default_string(settings, SETTING_HOTKEY, "");
	obs_data_set_default_int(settings, SETTING_MODE, MODE_TOGGLE);
}

struct filter_hotkey_property_context {
	obs_property_t *target_list;
	obs_source_t *controller;
	const char *current_target;
	bool current_seen;
};

static void filter_hotkey_add_filter_to_list(obs_source_t *parent, obs_source_t *filter, void *param)
{
	struct filter_hotkey_property_context *context = param;
	const char *filter_id;
	const char *filter_name;

	UNUSED_PARAMETER(parent);

	if (filter == context->controller)
		return;

	filter_id = obs_source_get_id(filter);
	if (filter_id && strcmp(filter_id, "filter_hotkey_controller") == 0)
		return;

	filter_name = obs_source_get_name(filter);
	if (!filter_name || !filter_name[0])
		return;

	obs_property_list_add_string(context->target_list, filter_name, filter_name);
	if (context->current_target && strcmp(context->current_target, filter_name) == 0)
		context->current_seen = true;
}

static obs_properties_t *filter_hotkey_properties(void *data_ptr)
{
	struct filter_hotkey_data *data = data_ptr;
	obs_properties_t *props = obs_properties_create();
	obs_property_t *target_list;
	obs_property_t *mode;
	obs_source_t *parent = NULL;
	char *current_target = NULL;
	struct filter_hotkey_property_context context;

	target_list = obs_properties_add_list(props, SETTING_TARGET_FILTER, "Filtro que sera controlado",
					      OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_STRING);
	obs_property_list_add_string(target_list, "Selecione um filtro da fonte", "");

	if (data) {
		filter_hotkey_mutex_lock(&data->mutex);
		current_target = bstrdup(filter_hotkey_get_target(data));
		filter_hotkey_mutex_unlock(&data->mutex);
		parent = obs_filter_get_parent(data->context);
	}

	context.target_list = target_list;
	context.controller = data ? data->context : NULL;
	context.current_target = current_target;
	context.current_seen = false;
	if (target_list && data)
		obs_property_set_modified_callback2(target_list, filter_hotkey_target_modified, data);
	if (parent)
		obs_source_enum_filters(parent, filter_hotkey_add_filter_to_list, &context);

	if (current_target && current_target[0] && !context.current_seen)
		obs_property_list_add_string(target_list, current_target, current_target);
	bfree(current_target);

	obs_property_t *hotkey_property = obs_properties_add_text(props, SETTING_HOTKEY,
									"Atalho (ex.: F8 ou Ctrl+Shift+F8)", OBS_TEXT_DEFAULT);
	if (hotkey_property && data)
		obs_property_set_modified_callback2(hotkey_property, filter_hotkey_hotkey_modified, data);
	obs_properties_add_button2(props, "save_hotkey", "Salvar bind", filter_hotkey_save_clicked, data);
	obs_properties_add_button2(props, "test_hotkey", "Testar acao agora", filter_hotkey_test_clicked, data);
	mode = obs_properties_add_list(props, SETTING_MODE, "Acao da tecla", OBS_COMBO_TYPE_LIST,
					OBS_COMBO_FORMAT_INT);
	obs_property_list_add_int(mode, "Alternar estado", MODE_TOGGLE);
	obs_property_list_add_int(mode, "Ativar filtro", MODE_ENABLE);
	obs_property_list_add_int(mode, "Desativar filtro", MODE_DISABLE);
	if (mode && data)
		obs_property_set_modified_callback2(mode, filter_hotkey_mode_modified, data);

	obs_properties_add_text(props, "hotkey_help",
				"Digite a combinação, clique em Salvar bind e use Testar acao agora para confirmar o alvo. Depois pressione a mesma tecla com o OBS aberto.",
				OBS_TEXT_INFO);
	obs_properties_add_text(props, "instructions",
				"O controlador deve estar na mesma fonte do filtro escolhido. Se a lista estiver vazia, adicione primeiro o filtro-alvo, salve, e abra as propriedades novamente.",
				OBS_TEXT_INFO);
	return props;
}

struct obs_source_info filter_hotkey_controller = {
	.id = "filter_hotkey_controller",
	.type = OBS_SOURCE_TYPE_FILTER,
	.output_flags = OBS_SOURCE_VIDEO,
	.get_name = filter_hotkey_get_name,
	.create = filter_hotkey_create,
	.destroy = filter_hotkey_destroy,
	.get_defaults = filter_hotkey_defaults,
	.get_properties = filter_hotkey_properties,
	.update = filter_hotkey_update,
	.filter_video = filter_hotkey_video,
	.video_render = filter_hotkey_render,
	.filter_add = filter_hotkey_add,
	.filter_remove = filter_hotkey_remove,
};
