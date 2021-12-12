#define _CRT_SECURE_NO_WARNINGS
#include "dns.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define CAPACITY 100000 //length of hash table

unsigned int hash_function(char* str) { // hash_table
    unsigned int i = 0;
    for (int j = 0; str[j]; j++) {
        i += str[j];
    }
    return i % CAPACITY;
}

typedef struct Ht_item Ht_item;

// Define the Hash Table Item here
struct Ht_item {
    char* key;
    IPADDRESS value;
};


typedef struct LinkedList LinkedList;

// Define the Linkedlist here
struct LinkedList {
    Ht_item* item;
    LinkedList* next;
};


typedef struct HashTable HashTable;


struct HashTable { // Contains an array of pointers to items
    Ht_item** items;
    LinkedList** overflow_buckets;
    int size;
    int count;
};


static LinkedList* allocate_list() {  // Allocates memory for a Linkedlist pointer
    LinkedList* list = (LinkedList*)malloc(sizeof(LinkedList));
    return list;
}

static LinkedList* linkedlist_insert(LinkedList* list, Ht_item* item) { // Inserts the item into the Linked List
    
    if (!list) {
        LinkedList* head = allocate_list();
        head->item = item;
        head->next = NULL;
        list = head;
        return list;
    }
    else if (list->next == NULL) {
        LinkedList* node = allocate_list();
        node->item = item;
        node->next = NULL;
        list->next = node;
        return list;
    }

    LinkedList* temp = list;
    LinkedList* temp_next = temp->next;
    while (temp->next != NULL) {
        temp = temp->next;
    }

    LinkedList* node = allocate_list();
    node->item = item;
    node->next = NULL;
    temp->next = node;

    return list;
}

static Ht_item* linkedlist_remove(LinkedList* list) { //delete linked list
    
    if (!list) {
        return NULL;
    }

    if (!list->next) {
        return NULL;
    }

    LinkedList* node = list->next;
    LinkedList* temp = list;
    temp->next = NULL;
    list = node;
    Ht_item* it = NULL;
    memcpy(temp->item, it, sizeof(Ht_item));
    free(temp->item->key);
    free(temp->item->value);
    free(temp->item);
    free(temp);
    return it;
}

static void free_linkedlist(LinkedList* list) {
    LinkedList* temp = list;
    while (list) {
        temp = list;
        list = list->next;
        free(temp->item->key);
        free(temp->item);
        free(temp);
    }
}

static LinkedList** create_overflow_buckets(HashTable* table) {   // Create the overflow buckets( an array of linkedlists )
    LinkedList** buckets = (LinkedList**)calloc(table->size, sizeof(LinkedList*));
    for (int i = 0; i < table->size; i++) {
        buckets[i] = NULL;
    }
    return buckets;
}

static void free_overflow_buckets(HashTable* table) {    // Free all the overflow bucket lists
    LinkedList** buckets = table->overflow_buckets;
    for (int i = 0; i < table->size; i++) {
        free_linkedlist(buckets[i]);
    }
    free(buckets);
}


Ht_item* create_item(char* key, IPADDRESS value) {  // Creates a pointer to a new hash table item
    Ht_item* item = (Ht_item*)malloc(sizeof(Ht_item));
    item->key = (char*)malloc(strlen(key) + 1);
    item->value = value;

    strcpy(item->key, key);

    return item;
}

HashTable* create_table(int size) { // Creates new HashTable
    HashTable* table = (HashTable*)malloc(sizeof(HashTable));
    table->size = size;
    table->count = 0;
    table->items = (Ht_item**)calloc(table->size, sizeof(Ht_item*));
    for (int i = 0; i < table->size; i++) {
        table->items[i] = NULL;
    }
    table->overflow_buckets = create_overflow_buckets(table);

    return table;
}

void free_item(Ht_item* item) {
    // Frees an item
    free(item->key);
    free(item);
}

void free_table(HashTable* table) {    // Frees the table
    for (int i = 0; i < table->size; i++) {
        Ht_item* item = table->items[i];
        if (item != NULL) {
            free_item(item);
        }
    }

    free_overflow_buckets(table);
    free(table->items);
    free(table);
}

void handle_collision(HashTable* table, unsigned long index, Ht_item* item) {
    LinkedList* head = table->overflow_buckets[index];
    table->overflow_buckets[index] = linkedlist_insert(head, item);
    return;

}

void ht_insert(HashTable* table, char* key, IPADDRESS value) {
    // Create item in hash table
    Ht_item* item = create_item(key, value);

    // Calculate index
    unsigned long index = hash_function(key);

    Ht_item* current_item = table->items[index];

    if (current_item == NULL) {
        // Key does not exist.
        if (table->count == table->size) {
            // Hash Table Full
            printf("Insert Error: Hash Table is full\n");
            // Remove the create item
            free_item(item);
            return;
        }

        // Insert directly
        table->items[index] = item;
        table->count++;
    }
    else {
        // update value
        if (strcmp(current_item->key, key) == 0) {
            table->items[index]->value = value;
            return;
        }
        else {
            // Collision
            handle_collision(table, index, item);
            return;
        }
    }
}

IPADDRESS ht_search(HashTable* table, char* key) {
    // Searches the key in the hashtable
    //  if key doesn't exist return NULL
    int index = hash_function(key);
    Ht_item* item = table->items[index];
    LinkedList* head = table->overflow_buckets[index];

    while (item != NULL) {
        if (strcmp(item->key, key) == 0) {
            return item->value;
        }
        if (head == NULL) {
            return NULL;
        }
        item = head->item;
        head = head->next;
    }

    return NULL;
}

HashTable** DNSTables[50];
DNSHandle DNSCounter = (DNSHandle)1;

DNSHandle InitDNS()
{
    DNSTables[DNSCounter] = create_table(CAPACITY);
    DNSCounter += 1;
    return DNSCounter - 1;
}

void LoadHostsFile(DNSHandle hDNS, const char* hostsFilePath)
{
    FILE* fInput = NULL;
    unsigned int dnCount = 0;
    unsigned int i = 0;

    fInput = fopen("hosts", "r");
    if (NULL == fInput) {
        return;
    }
    dnCount = getNumOfLines(fInput);

    if (0 == dnCount){
        fclose(fInput);
        return;
    }

    fseek(fInput, 0, SEEK_SET);


    for (i = 0; i < dnCount && !feof(fInput); i++)
    {
        // Goes through hosts
        char buffer[201] = { 0 };
        char* pStringWalker = &buffer[0];
        unsigned int uHostNameLength = 0;
        unsigned int ip1 = 0, ip2 = 0, ip3 = 0, ip4 = 0;

        fgets(buffer, 200, fInput);

        if (5 != fscanf_s(fInput, "%d.%d.%d.%d %s", &ip1, &ip2, &ip3, &ip4, buffer, 200)) {
            continue;
        }
        // Create int ip out of 4 sections
        IPADDRESS ip = (ip1 & 0xFF) << 24 |
            (ip2 & 0xFF) << 16 |
            (ip3 & 0xFF) << 8 |
            (ip4 & 0xFF);

        char* domainName;
        uHostNameLength = strlen(buffer);
        if (uHostNameLength){
            domainName = (char*)malloc(uHostNameLength + 1);
            strcpy(domainName, pStringWalker);
            // Insert into hash table
            ht_insert(DNSTables[hDNS], domainName, ip);
        }
    }
    fclose(fInput);
}

IPADDRESS DnsLookUp(DNSHandle hDNS, const char* hostName)
{
    IPADDRESS result = ht_search(DNSTables[hDNS], hostName);
    return result;
}

void ShutdownDNS(DNSHandle hDNS)
{
    free_table(DNSTables[hDNS]);
}