/*
 * Copyright (c) 1997 Kungliga Tekniska H�gskolan
 * (Royal Institute of Technology, Stockholm, Sweden). 
 * All rights reserved. 
 *
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions 
 * are met: 
 *
 * 1. Redistributions of source code must retain the above copyright 
 *    notice, this list of conditions and the following disclaimer. 
 *
 * 2. Redistributions in binary form must reproduce the above copyright 
 *    notice, this list of conditions and the following disclaimer in the 
 *    documentation and/or other materials provided with the distribution. 
 *
 * 3. All advertising materials mentioning features or use of this software 
 *    must display the following acknowledgement: 
 *      This product includes software developed by Kungliga Tekniska 
 *      H�gskolan and its contributors. 
 *
 * 4. Neither the name of the Institute nor the names of its contributors 
 *    may be used to endorse or promote products derived from this software 
 *    without specific prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND 
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE 
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL 
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS 
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) 
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT 
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY 
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF 
 * SUCH DAMAGE. 
 */

#include "kadm5_locl.h"

RCSID("$Id$");

static struct units acl_units[] = {
    { "list",	KADM5_ACL_LIST },
    { "delete",	KADM5_ACL_DELETE },
    { "chpass",	KADM5_ACL_CHPASS },
    { "modify",	KADM5_ACL_MODIFY },
    { "add",	KADM5_ACL_CREATE },
    { "get", 	KADM5_ACL_GET },
    { NULL }
};

kadm5_ret_t
_kadm5_acl_init(kadm5_server_context *context)
{
    FILE *f;
    char buf[128];
    krb5_principal princ;
    int flags;
    krb5_error_code ret;
    
    krb5_parse_name(context->context, KADM5_ADMIN_SERVICE, &princ);
    if(krb5_principal_compare(context->context, context->caller, princ)){
	krb5_free_principal(context->context, princ);
	context->acl_flags = ~0;
	return 0;
    }

    flags = -1;
    f = fopen(context->acl_file, "r");
    if(f){
	while(fgets(buf, sizeof(buf), f)){
	    char *foo = NULL, *p;
	    p = strtok_r(buf, " \t\n", &foo);
	    if(p == NULL)
		continue;
	    ret = krb5_parse_name(context->context, p, &princ);
	    if(ret)
		continue;
	    if(!krb5_principal_compare(context->context, 
				       context->caller,  princ)){
		krb5_free_principal(context->context, princ);
		continue;
	    }
	    krb5_free_principal(context->context, princ);
	    p = strtok_r(NULL, "\n", &foo);
	    if(p == NULL)
		continue;
	    flags = parse_flags(p, acl_units, 0);
	    break;
	}
	fclose(f);
    }
    if(flags == -1)
	flags = 0;
    context->acl_flags = flags;
    return 0;
}

kadm5_ret_t
_kadm5_acl_check_permission(kadm5_server_context *context, unsigned op)
{
    unsigned res = ~context->acl_flags & op;
    if(res & KADM5_ACL_GET)
	return KADM5_AUTH_GET;
    if(res & KADM5_ACL_CREATE)
	return KADM5_AUTH_ADD;
    if(res & KADM5_ACL_MODIFY)
	return KADM5_AUTH_MODIFY;
    if(res & KADM5_ACL_DELETE)
	return KADM5_AUTH_DELETE;
    if(res & KADM5_ACL_CHPASS)
	return KADM5_AUTH_CHANGEPW;
    if(res & KADM5_ACL_LIST)
	return KADM5_AUTH_LIST;
    if(res)
	return KADM5_AUTH_INSUFFICIENT;
    return 0;
}
