/*******************************************************************//**
 *	@file		test_slist.c
 *
 *	Test cgi slist implementation.
 *
 *	@author		Alexander Dahl <post@lespocky.de>
 *
 *	SPDX-License-Identifier: LGPL-2.1+
 *	License-Filename: LICENSES/LGPL-2.1.txt
 *
 *	@copyright	2016 Alexander Dahl <post@lespocky.de>
 **********************************************************************/

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "cgi_test.h"

#include "libcgi/cgi.h"
#include "libcgi/cgi_types.h"

/*	declarations for functions not declared in src	*/
formvars *process_data(const char *query, formvars **start, formvars **last,
	                   const char sep_value, const char sep_name);

/*	local declarations	*/
static int add( void );
static int delete_from_empty_list( void );
static int delete_from_one_item_list( void );
static int delete_from_two_item_list( void );
static int delete_from_three_item_list( void );
static int get_item( void );
static int test_process_data( void );

int main( int argc, char *argv[] )
{
	struct cgi_test_action	actions[] = {
		{ "add",			add							},
		{ "deletezero",		delete_from_empty_list		},
		{ "deleteone",		delete_from_one_item_list	},
		{ "deletetwo",		delete_from_two_item_list	},
		{ "deletethree",	delete_from_three_item_list	},
		{ "get",			get_item					},
		{ "processdata",	test_process_data			},
	};

	/*	require at least one argument to select test	*/
	if ( argc < 2 ) return EXIT_FAILURE;

	return run_action( argv[1], actions,
			sizeof(actions)/sizeof(struct cgi_test_action) );
}

int add( void )
{
	int			i;
	formvars	*item[2] = { NULL, NULL }, *start = NULL, *last = NULL;

	check( (item[0] = calloc( 1, sizeof(formvars) )) != NULL, "calloc item 0" );
	check( (item[0]->name = strdup( "item 0" )) != NULL, "strdup name item 0" );
	check( (item[0]->value = strdup( "zero" )) != NULL, "strdup value item 0" );

	slist_add( item[0], &start, &last );
	check( start == item[0], "start == first item" );
	check( last == item[0], "last == first item" );
	check( item[0]->next == NULL, "first item next == null" );

	check( (item[1] = calloc( 1, sizeof(formvars) )) != NULL, "calloc item 1" );
	check( (item[1]->name = strdup( "item 1" )) != NULL, "strdup name item 1" );
	check( (item[1]->value = strdup( "one" )) != NULL, "strdup value item 1" );
	slist_add( item[1], &start, &last );
	check( start == item[0], "start == first item" );
	check( last == item[1], "last == first item" );
	check( item[0]->next == item[1], "first item next == item 1" );
	check( item[1]->next == NULL, "second item next == null" );

	/*	TODO	add tests for use with wrong arguments!	*/

	slist_free( &start );
	check( start == NULL, "slist_free reset start" );

	return EXIT_SUCCESS;

error:
	for ( i = 0; i < 2; i++ ) {
		if ( item[i]->name ) free( item[i]->name );
		if ( item[i]->value ) free( item[i]->value );
		if ( item[i] ) free( item[i] );
	}
	return EXIT_FAILURE;
}

int delete_from_empty_list( void )
{
	formvars	*start = NULL, *last = NULL;

	/*	add no item, try to delete an item	*/
	check( slist_delete( "not in there", &start, &last ) == 0, "not found" );

	return EXIT_SUCCESS;

error:
	return EXIT_FAILURE;
}

int delete_from_one_item_list( void )
{
	const char	item_name[] = "item";
	formvars	*item, *start = NULL, *last = NULL;

	/*	add one item, delete one item	*/
	check( (item = calloc( 1, sizeof(formvars) )) != NULL, "calloc item" );
	check( (item->name = strdup( item_name )) != NULL, "strdup name item" );
	check( (item->value = strdup( "single" )) != NULL, "strdup value item" );
	slist_add( item, &start, &last );

	/*	try deleting not existing item and check list does not change	*/
	check( slist_delete( "not in there", &start, &last ) == 0, "not found" );
	check( start == item, "start == first item" );
	check( last == item, "last == first item" );
	check( item->next == NULL, "first item next == null" );

	/*	try deleting our item	*/
	check( slist_delete( item_name, &start, &last ) == 1, "found" );
	check( start == NULL, "start = null" );
	check( last == NULL, "last = null" );
	item = NULL;

	/*	the list should already be empty	*/
	slist_free( &start );
	check( start == NULL, "slist_free reset start" );

	return EXIT_SUCCESS;

error:
	if ( item->name ) free( item->name );
	if ( item->value ) free( item->value );
	if ( item ) free( item );
	return EXIT_FAILURE;
}

int delete_from_two_item_list( void )
{
	int			i;
	const char	*item_name[2] = { "item_one", "item_two" };
	formvars	*item[2], *start = NULL, *last = NULL;

	for ( i = 0; i < 2; i++ )
	{
		item[i] = NULL;
	}

	for ( i = 0; i < 2; i++ )
	{
		check( (item[i] = calloc( 1, sizeof(formvars) )) != NULL,
				"calloc item %i", i );
		check( (item[i]->name = strdup( item_name[i] )) != NULL,
				"strdup name item %i", i );
		check( (item[i]->value = strdup( "value" )) != NULL,
				"strdup value item %i", i );
		slist_add( item[i], &start, &last );
	}

	/*	try deleting not existing item and check list does not change	*/
	check( slist_delete( "not in there", &start, &last ) == 0, "not found" );
	check( start == item[0], "start == item[0]" );
	check( last == item[1], "last == item[1]" );
	check( item[0]->next == item[1], "first item next == second item" );
	check( item[1]->next == NULL, "second item next == null" );

	/*	empty list	*/
	slist_free( &start );
	check( start == NULL, "slist_free reset start" );

	/*	start over	*/
	for ( i = 0; i < 2; i++ )
	{
		item[i] = NULL;
	}

	for ( i = 0; i < 2; i++ )
	{
		check( (item[i] = calloc( 1, sizeof(formvars) )) != NULL,
				"calloc item %i", i );
		check( (item[i]->name = strdup( item_name[i] )) != NULL,
				"strdup name item %i", i );
		check( (item[i]->value = strdup( "value" )) != NULL,
				"strdup value item %i", i );
		slist_add( item[i], &start, &last );
	}

	/*	try deleting the first item	*/
	check( slist_delete( item_name[0], &start, &last ) == 1,
			"delete first item" );
	check( start == item[1], "start == item[1]" );
	check( last == item[1], "last == item[1]" );
	check( item[1]->next == NULL, "last item next == null" );

	/*	empty list	*/
	slist_free( &start );
	check( start == NULL, "slist_free reset start" );

	/*	start over	*/
	for ( i = 0; i < 2; i++ )
	{
		item[i] = NULL;
	}

	for ( i = 0; i < 2; i++ )
	{
		check( (item[i] = calloc( 1, sizeof(formvars) )) != NULL,
				"calloc item %i", i );
		check( (item[i]->name = strdup( item_name[i] )) != NULL,
				"strdup name item %i", i );
		check( (item[i]->value = strdup( "value" )) != NULL,
				"strdup value item %i", i );
		slist_add( item[i], &start, &last );
	}

	/*	try deleting the second item	*/
	check( slist_delete( item_name[1], &start, &last ) == 1,
			"delete second item" );
	check( start == item[0], "start == item[0]" );
	check( last == item[0], "last == item[0]" );
	check( item[0]->next == NULL, "last item next == null" );

	/*	empty list	*/
	slist_free( &start );
	check( start == NULL, "slist_free reset start" );

	return EXIT_SUCCESS;

error:
	for ( i = 0; i < 2; i++ )
	{
		if ( item[i]->name ) free( item[i]->name );
		if ( item[i]->value ) free( item[i]->value );
		if ( item[i] ) free( item[i] );
	}
	return EXIT_FAILURE;
}

int delete_from_three_item_list( void )
{
	int			i;
	const char	*item_name[3] = { "item_one", "item_two", "item_three" };
	formvars	*item[3], *start = NULL, *last = NULL;

	for ( i = 0; i < 3; i++ )
	{
		item[i] = NULL;
	}

	for ( i = 0; i < 3; i++ )
	{
		check( (item[i] = calloc( 1, sizeof(formvars) )) != NULL,
				"calloc item %i", i );
		check( (item[i]->name = strdup( item_name[i] )) != NULL,
				"strdup name item %i", i );
		check( (item[i]->value = strdup( "value" )) != NULL,
				"strdup value item %i", i );
		slist_add( item[i], &start, &last );
	}

	check( start == item[0], "start == item[0]" );
	check( last == item[2], "last == item[2]" );
	check( item[0]->next == item[1], "item[0]->next == item[1]" );
	check( item[1]->next == item[2], "item[1]->next == item[2]" );
	check( item[2]->next == NULL, "item[2]->next == NULL" );

	/*	try deleting the first item	*/
	check( slist_delete( item_name[0], &start, &last ) == 1,
			"delete item[0]" );
	check( start == item[1], "start == item[1]" );
	check( last == item[2], "last == item[2]" );
	check( item[1]->next == item[2], "item[1]->next == item[2]" );
	check( item[2]->next == NULL, "item[2]->next == NULL" );

	/*	empty list	*/
	slist_free( &start );
	check( start == NULL, "slist_free reset start" );

	/*	start over	*/
	for ( i = 0; i < 3; i++ )
	{
		item[i] = NULL;
	}

	for ( i = 0; i < 3; i++ )
	{
		check( (item[i] = calloc( 1, sizeof(formvars) )) != NULL,
				"calloc item %i", i );
		check( (item[i]->name = strdup( item_name[i] )) != NULL,
				"strdup name item %i", i );
		check( (item[i]->value = strdup( "value" )) != NULL,
				"strdup value item %i", i );
		slist_add( item[i], &start, &last );
	}

	/*	try deleting the middle item	*/
	check( slist_delete( item_name[1], &start, &last ) == 1,
			"delete item[1]" );
	check( start == item[0], "start == item[0]" );
	check( last == item[2], "last == item[2]" );
	check( item[0]->next == item[2], "item[0]->next == item[2]" );
	check( item[2]->next == NULL, "item[2]->next == NULL" );

	/*	empty list	*/
	slist_free( &start );
	check( start == NULL, "slist_free reset start" );

	/*	start over	*/
	for ( i = 0; i < 3; i++ )
	{
		item[i] = NULL;
	}

	for ( i = 0; i < 3; i++ )
	{
		check( (item[i] = calloc( 1, sizeof(formvars) )) != NULL,
				"calloc item %i", i );
		check( (item[i]->name = strdup( item_name[i] )) != NULL,
				"strdup name item %i", i );
		check( (item[i]->value = strdup( "value" )) != NULL,
				"strdup value item %i", i );
		slist_add( item[i], &start, &last );
	}

	/*	try deleting the last item	*/
	check( slist_delete( item_name[2], &start, &last ) == 1,
			"delete item[2]" );
	check( start == item[0], "start == item[0]" );
	check( last == item[1], "last == item[1]" );
	check( item[0]->next == item[1], "item[0]->next == item[1]" );
	check( item[1]->next == NULL, "item[1]->next == NULL" );

	/*	empty list	*/
	slist_free( &start );
	check( start == NULL, "slist_free reset start" );

	return EXIT_SUCCESS;

error:
	for ( i = 0; i < 3; i++ )
	{
		if ( item[i]->name ) free( item[i]->name );
		if ( item[i]->value ) free( item[i]->value );
		if ( item[i] ) free( item[i] );
	}
	return EXIT_FAILURE;
}

int get_item( void )
{
	char		*my_item;
	int			i;
	const char	*item_name[4] = { "item_one", "item_two", "item_three",
						"item_four" };
	formvars	*item[4], *start = NULL, *last = NULL;

	for ( i = 0; i < 4; i++ )
	{
		item[i] = NULL;
	}

	/*	init three items and add all of them to our list	*/
	for ( i = 0; i < 4; i++ )
	{
		check( (item[i] = calloc( 1, sizeof(formvars) )) != NULL,
				"calloc item %i", i );
		check( (item[i]->name = strdup( item_name[i] )) != NULL,
				"strdup name item %i", i );
		if ( i != 2 ) {
			check( (item[i]->value = strdup( "value" )) != NULL,
					"strdup value item %i", i );
		} else {
			/*	item[i]->value == NULL;	*/
		}
		slist_add( item[i], &start, &last );
	}

	/*	good case, try retrieving the actual items	*/
	check( (my_item = slist_item( item_name[0], start ))
			== item[0]->value, "get item[0]" );
	check( (my_item = slist_item( item_name[1], start ))
			== item[1]->value, "get item[1]" );
	check( (my_item = slist_item( item_name[3], start ))
			== item[3]->value, "get item[3]" );

	/*	try getting a non existing item	*/
	check( (my_item = slist_item( "not", start )) == NULL,
			"not existing item" );

	/*	try bad parameters	*/
	check( (my_item = slist_item( "not", NULL )) == NULL, "start == NULL" );
	check( (my_item = slist_item( NULL, start )) == NULL, "name == NULL" );

	/*	try getting the item with the empty value	*/
	check( (my_item = slist_item( item_name[2], start )) == NULL,
			"value == NULL" );

	/*	empty list	*/
	slist_free( &start );
	check( start == NULL, "slist_free reset start" );

	return EXIT_SUCCESS;

error:
	for ( i = 0; i < 3; i++ )
	{
		if ( item[i]->name ) free( item[i]->name );
		if ( item[i]->value ) free( item[i]->value );
		if ( item[i] ) free( item[i] );
	}
	return EXIT_FAILURE;
}

int test_process_data( void )
{
	const char	query[] = "foo=bar&bar=baz";
	char		*item;
	formvars	*r, *start = NULL, *last = NULL;

	/*	simple test for the good case	*/
	check( (r = process_data( query, &start, &last, '=', '&' ))
			== start, "return value: %p (start: %p)", r, start );
	check( (item = slist_item( "foo", start )) != NULL, "get item foo" );
	check( strcmp( item, "bar" ) == 0, "value item foo" );

	/*	TODO	add more tests for corner cases, wrong input, and so on	*/

	/*	empty list	*/
	slist_free( &start );
	check( start == NULL, "slist_free reset start" );

	return EXIT_SUCCESS;

error:
	return EXIT_FAILURE;
}

/* vim: set noet sts=0 ts=4 sw=4 sr: */
