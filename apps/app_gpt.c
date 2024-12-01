/*
 * Asterisk -- An open source telephony toolkit.
 *
 * Copyright (C) 2024, Maksim Nesterov
 *
 * Maksim Nesterov <braamsdev@gmail.com>
 *
 * See http://www.asterisk.org for more information about
 * the Asterisk project. Please do not directly contact
 * any of the maintainers of this project for assistance;
 * the project provides a web site, mailing lists and IRC
 * channels for your use.
 *
 * This program is free software, distributed under the terms of
 * the GNU General Public License Version 2. See the LICENSE file
 * at the top of the source tree.
 *
 * Please follow coding guidelines
 * https://docs.asterisk.org/Development/Policies-and-Procedures/Coding-Guidelines/
 */

/*! \file
 *
 * \brief GPT application
 *
 * \author\verbatim Maksim Nesterov <braamsdev@gmail.com> \endverbatim
 *
 * This is an application for communication with the OpenAI realtime APY
 * \ingroup applications
 */

/*! \li \ref app_gpt.c uses configuration file \ref app_gpt.conf
 * \addtogroup configuration_file Configuration Files
 */

/*!
 * \page app_gpt.conf app_gpt.conf
 * \verbinclude app_gpt.conf.sample
 */

/*** MODULEINFO
	<defaultenabled>no</defaultenabled>
	<support_level>core</support_level>
 ***/

#include "asterisk.h"

#include "asterisk/file.h"
#include "asterisk/channel.h"
#include "asterisk/module.h"
#include "asterisk/lock.h"
#include "asterisk/app.h"
#include "asterisk/netsock2.h"
#include "asterisk/strings.h"

static char *app = "GPT";

static int app_exec(struct ast_channel *chan, const char *data) {

    char *encoded_buffer;
    char *json_buffer;
    struct ast_json *append_json;

    while (ast_waitfor(chan, -1) > -1) {
        struct ast_frame *f = ast_read(chan);
        if (!f) {
            break;
        }

        f->delivery.tv_sec = 0;
        f->delivery.tv_usec = 0;
        if (f->frametype == AST_FRAME_VOICE) {

            encoded_buffer = ast_malloc(f->datalen * 2);
            ast_base64encode(encoded_buffer, (unsigned char *) f->data.ptr,
                             f->datalen, f->datalen * 2);
            append_json = ast_json_pack("{s:s, s:s}",
                                        "type", "input_audio_buffer.append",
                                        "audio", encoded_buffer);
            json_buffer = ast_json_dump_string(append_json);
            ast_json_unref(append_json);
            ast_verb(2, "send: %s\n", json_buffer);
            ast_free(encoded_buffer);
        }
    }
    return 0;
}

static int reload_module(void) {
    return 0;
}

static int unload_module(void) {
    return ast_unregister_application(app);
}

static int load_module(void) {
    ast_register_application_xml(app, app_exec);
    return AST_MODULE_LOAD_SUCCESS;
}

AST_MODULE_INFO(ASTERISK_GPL_KEY, AST_MODFLAG_DEFAULT,
                "GPT Application", .support_level = AST_MODULE_SUPPORT_CORE, .load = load_module,
                .unload = unload_module, .reload = reload_module,);
