/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details:
 *
 * Copyright (C) 2019 Aleksander Morgado <aleksander@aleksander.es>
 */

#include <config.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "ModemManager.h"
#define _LIBMM_INSIDE_MM
#include <libmm-glib.h>
#include "mm-log.h"
#include "mm-errors-types.h"
#include "mm-modem-helpers-simtech.h"
#include "mm-modem-helpers.h"


/*****************************************************************************/
/* +CLCC test parser
 *
 * Example (SIM7600E):
 *   AT+CLCC=?
 *   +CLCC: (0-1)
 */

gboolean
mm_simtech_parse_clcc_test (const gchar  *response,
                            gboolean     *clcc_urcs_supported,
                            GError      **error)
{
    g_assert (response);

    response = mm_strip_tag (response, "+CLCC:");

    /* 3GPP specifies that the output of AT+CLCC=? should be just OK, so support
     * that */
    if (!response[0]) {
        *clcc_urcs_supported = FALSE;
        return TRUE;
    }

    /* As per 3GPP TS 27.007, the AT+CLCC command doesn't expect any argument,
     * as it only is designed to report the current call list, nothing else.
     * In the case of the Simtech plugin, though, we are going to support +CLCC
     * URCs that can be enabled/disabled via AT+CLCC=1/0. We therefore need to
     * detect whether this URC management is possible or not, for now with a
     * simple check looking for the specific "(0-1)" string.
     */
    if (!strncmp (response, "(0-1)", 5)) {
        *clcc_urcs_supported = TRUE;
        return TRUE;
    }

    g_set_error (error, MM_CORE_ERROR, MM_CORE_ERROR_FAILED,
                 "unexpected +CLCC test response: '%s'", response);
    return FALSE;
}

/*****************************************************************************/

GRegex *
mm_simtech_get_clcc_urc_regex (void)
{
    return g_regex_new ("\\r\\n(\\+CLCC: .*\\r\\n)+",
                        G_REGEX_RAW | G_REGEX_OPTIMIZE, 0, NULL);
}

gboolean
mm_simtech_parse_clcc_list (const gchar *str,
                            GList      **out_list,
                            GError     **error)
{
    /* Parse the URC contents as a plain +CLCC response, but make sure to skip first
     * EOL in the string because the plain +CLCC response would never have that.
     */
    return mm_3gpp_parse_clcc_response (mm_strip_tag (str, "\r\n"), out_list, error);
}

void
mm_simtech_call_info_list_free (GList *call_info_list)
{
    mm_3gpp_call_info_list_free (call_info_list);
}