#if 0
#include <bson.h>
#include <mongoc.h>
#include <stdio.h>

int
main (int   argc,
      char *argv[])
{
    mongoc_client_t *client;
    mongoc_collection_t *collection;
    mongoc_cursor_t *cursor;
    bson_error_t error;
    bson_oid_t oid;
    bson_t *doc;

    mongoc_init ();

    client = mongoc_client_new ("mongodb://127.0.0.1:10001/");
    collection = mongoc_client_get_collection (client, "test2", "test");

    doc = bson_new ();
    bson_oid_init (&oid, NULL);
    BSON_APPEND_OID (doc, "_id", &oid);
    BSON_APPEND_UTF8 (doc, "hello", "world");

    if (!mongoc_collection_insert (collection, MONGOC_INSERT_NONE, doc, NULL, &error)) {
        printf ("Insert failed: %s\n", error.message);
    }

    bson_destroy (doc);
/*
    doc = bson_new ();
    BSON_APPEND_OID (doc, "_id", &oid);

    if (!mongoc_collection_delete (collection, MONGOC_DELETE_SINGLE_REMOVE, doc, NULL, &error)) {
        printf ("Delete failed: %s\n", error.message);
    }

    bson_destroy (doc);
*/
    mongoc_collection_destroy (collection);
    mongoc_client_destroy (client);

    return 0;
}
#endif

#include <bson.h>
#include <bcon.h>
#include <mongoc.h>
#include <stdio.h>

static void
run_command (void)
{
    mongoc_client_t *client;
    mongoc_collection_t *collection;
    bson_error_t error;
    bson_t *command;
    bson_t reply;
    char *str;

    client = mongoc_client_new ("mongodb://127.0.0.1:10001/");
    collection = mongoc_client_get_collection (client, "test", "test");

    command = BCON_NEW ("collStats", BCON_UTF8 ("test"));
    if (mongoc_collection_command_simple (collection, command, NULL, &reply, &error)) {
        str = bson_as_json (&reply, NULL);
        printf ("%s\n", str);
        bson_free (str);
    } else {
        fprintf (stderr, "Failed to run command: %s\n", error.message);
    }

    bson_destroy (command);
    bson_destroy (&reply);
    mongoc_collection_destroy (collection);
    mongoc_client_destroy (client);
}

int
main (int   argc,
      char *argv[])
{
    mongoc_init ();
    run_command ();
    mongoc_cleanup ();

    return 0;
}



