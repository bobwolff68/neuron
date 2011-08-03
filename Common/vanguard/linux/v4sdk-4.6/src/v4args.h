
/**
 * @file v4args.h
 * command line arguments parsing routines
 *
 * Project:	VSS H.264 Codec
 *
 *
 * (c) Vanguard Software Solutions Inc 1995-2010, All Rights Reserved
 * This document and software are the intellectual property of Vanguard Software Solutions Inc. (VSofts)
 * Your use is governed by the terms of the license agreement between you and VSofts.
 * All other uses and/or modifications are strictly prohibited.
 */

#ifndef __V4ARGS_H__
#define __V4ARGS_H__

#include "v4file.h"

//////////////////////////////////////////////////////////////////////////
enum
{
	ARG_TYPE_INT   = 0,
	ARG_TYPE_CHAR  = 1,
	ARG_TYPE_FLOAT = 2,
};

typedef struct cmd_arg_t
{
	char *name;
	char *alias;
	char *long_name;
	int   type;
	void *pvalue;
	int   def_value;
	char *note;
} cmd_arg_t;

typedef struct arg_ctx_t
{
	cmd_arg_t *args;
	int cnt;
} arg_ctx_t;

static void read_args(arg_ctx_t *ctx, int argc, char *argv[]);
static void print_args(arg_ctx_t *ctx);
static void dump_args(arg_ctx_t *ctx);


typedef void read_args_func_t(void *cb, cmd_arg_t *arg, char *value);
static void read_args_notify(arg_ctx_t *ctx, int argc, char *argv[], void *cb, read_args_func_t *func);

//////////////////////////////////////////////////////////////////////////
static cmd_arg_t *find_arg(arg_ctx_t *ctx, char *name)
{
	cmd_arg_t *arg = NULL;
	int i, dash_num;
	dash_num = 0;
	while ((strlen(name) > 1) && (name[0] == '-'))
	{
		dash_num += 1;
		name += 1;
	}
	if (dash_num > 0)
	{
		for (i=0; i<ctx->cnt; i++)
		{
			if ((strcmp(ctx->args[i].long_name, name) == 0) ||
				(strcmp(ctx->args[i].alias, name) == 0) ||
				(strcmp(ctx->args[i].name, name) == 0))
			{
				arg = &ctx->args[i];
				break;
			}
		}
	}
	return arg;
}

static void set_arg_value(cmd_arg_t *arg, char *value, void *cb, read_args_func_t *func)
{
	if (!arg) return;
	switch (arg->type)
	{
	case ARG_TYPE_CHAR:
		{
			char **pchar = (char**)arg->pvalue;
			if (value && (strlen(value) > 0) && (value[0] != '-'))
			{
				*pchar = value;
			}
		}
		break;
	case ARG_TYPE_INT:
		{
			int *pint = (int*)arg->pvalue;
			int ival = value? atoi(value) : arg->def_value;
			*pint = ival;
		}
		break;
	case ARG_TYPE_FLOAT:
		{
			double *pdbl = (double*)arg->pvalue;
			double dval = value? atof(value) : (double)arg->def_value;
			*pdbl = dval;
		}
		break;
	}
	if (func)
		func(cb, arg, value);
}

static void read_args_notify(arg_ctx_t *ctx, int argc, char *argv[], void *cb, read_args_func_t *func)
{
	int i;
	cmd_arg_t *arg;
	cmd_arg_t *prev_arg;
	char *prev_val;
	prev_arg = NULL;
	prev_val = NULL;
	for (i=1; i<argc; i++)
	{
		// find appropriate argument in array
		arg = find_arg(ctx, argv[i]);
		if (arg)
		{
			set_arg_value(prev_arg, prev_val, cb, func);
			prev_arg = arg;
			prev_val = NULL;
		}
		else
		{
			if (!prev_val)
				prev_val = argv[i];
		}
	}
	set_arg_value(prev_arg, prev_val, cb, func);
}

static void read_args(arg_ctx_t *ctx, int argc, char *argv[])
{
	read_args_notify(ctx, argc, argv, NULL, NULL);
}

static void print_args(arg_ctx_t *ctx)
{
	cmd_arg_t *arg;
	int i;
	for (i=0; i<ctx->cnt; i++)
	{
		arg = &ctx->args[i];
		verbose_print(0, "-%s (-%s): [-%s] %s;\n", arg->name, arg->alias, arg->long_name, arg->note);
	}
}

static void dump_args(arg_ctx_t *ctx)
{
	cmd_arg_t *arg;
	int i;
	for (i=0; i<ctx->cnt; i++)
	{
		arg = &ctx->args[i];
		switch (arg->type)
		{
		case ARG_TYPE_CHAR:
			{
				char *sval = *(char**)arg->pvalue;
				if (sval) 
					verbose_print(0, "-%s (-%s) = [%s] %s\n", arg->name, arg->alias, arg->long_name, sval);
			}
			break;
		case ARG_TYPE_INT:
			{
				int ival = *(int*)arg->pvalue;
				verbose_print(0, "-%s (-%s) = [%s] %d\n", arg->name, arg->alias, arg->long_name, ival);
			}
			break;
		case ARG_TYPE_FLOAT:
			{
				double dval = *(double *)arg->pvalue;
				verbose_print(0, "-%s (-%s) = [%s] %g\n", arg->name, arg->alias, arg->long_name, dval);
			}
			break;
		}
	}
}
//////////////////////////////////////////////////////////////////////////

#endif	//__V4ARGS_H__
