
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>

#define INVENTORY_FILE "inventory.txt"
#define TEMP_FILE "inventory.tmp"
#define MAX_NAME 100

void clearInput(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

int readPositiveInt(const char *message, int *number) {
    char input[100];
    char *end;
    long value;

    printf("%s", message);

    if (fgets(input, sizeof(input), stdin) == NULL)
        return 0;

    value = strtol(input, &end, 10);

    while (isspace((unsigned char)*end))
        end++;

    if (end == input || *end != '\0' ||
        value <= 0 || value > INT_MAX)
        return 0;

    *number = (int)value;
    return 1;
}

int readItemName(char *name, size_t size) {
    printf("Enter the item name: ");

    if (fgets(name, size, stdin) == NULL)
        return 0;

    if (strchr(name, '\n') == NULL && !feof(stdin)) {
        clearInput();
        return 0;
    }

    name[strcspn(name, "\r\n")] = '\0';

    return strlen(name) > 0;
}

int readRecord(FILE *file, char *name, size_t size, int *quantity) {
    char quantityLine[100];
    char *end;
    long value;

    if (fgets(name, size, file) == NULL)
        return 0;

    if (strchr(name, '\n') == NULL && !feof(file))
        return -1;

    name[strcspn(name, "\r\n")] = '\0';

    if (fgets(quantityLine, sizeof(quantityLine), file) == NULL)
        return -1;

    value = strtol(quantityLine, &end, 10);

    while (isspace((unsigned char)*end))
        end++;

    if (end == quantityLine || *end != '\0' ||
        value < 0 || value > INT_MAX)
        return -1;

    *quantity = (int)value;
    return 1;
}

void addItem(void) {
    FILE *file;
    char name[MAX_NAME];
    int quantity;

    if (!readItemName(name, sizeof(name))) {
        printf("Invalid item name!\n");
        return;
    }

    if (!readPositiveInt("Enter the quantity: ", &quantity)) {
        printf("Invalid quantity!\n");
        return;
    }

    file = fopen(INVENTORY_FILE, "a");

    if (file == NULL) {
        printf("Error opening inventory file!\n");
        return;
    }

    fprintf(file, "%s\n%d\n", name, quantity);
    fclose(file);

    printf("Item added successfully!\n");
}

void removeItem(void) {
    FILE *file, *temp;
    char name[MAX_NAME];
    char storedName[MAX_NAME];

    int quantity, removeQuantity;
    int availableQuantity = 0;
    int found = 0;
    int status = 0;
    int result;

    if (!readItemName(name, sizeof(name))) {
        printf("Invalid item name!\n");
        return;
    }

    if (!readPositiveInt("Enter the quantity to be removed: ",
                         &removeQuantity)) {
        printf("Invalid quantity!\n");
        return;
    }

    file = fopen(INVENTORY_FILE, "r");

    if (file == NULL) {
        printf("Item not found.\n");
        return;
    }

    temp = fopen(TEMP_FILE, "w");

    if (temp == NULL) {
        fclose(file);
        printf("Error creating temporary file!\n");
        return;
    }

    while ((result = readRecord(file, storedName,
                                sizeof(storedName), &quantity)) == 1) {

        if (!found && strcmp(name, storedName) == 0) {
            found = 1;
            availableQuantity = quantity;

            if (removeQuantity > quantity) {
                status = 3;
                fprintf(temp, "%s\n%d\n", storedName, quantity);
            }
            else if (removeQuantity == quantity) {
                status = 2;
            }
            else {
                status = 1;
                fprintf(temp, "%s\n%d\n",
                        storedName, quantity - removeQuantity);
            }
        }
        else {
            fprintf(temp, "%s\n%d\n", storedName, quantity);
        }
    }

    fclose(file);
    fclose(temp);

    if (result == -1) {
        remove(TEMP_FILE);
        printf("Error reading inventory file!\n");
        return;
    }

    if (!found) {
        remove(TEMP_FILE);
        printf("Item not found.\n");
        return;
    }

    if (status == 3) {
        remove(TEMP_FILE);
        printf("Insufficient stock. Available quantity: %d\n",
               availableQuantity);
        return;
    }

    if (remove(INVENTORY_FILE) != 0) {
        remove(TEMP_FILE);
        printf("Error updating inventory file!\n");
        return;
    }

    if (rename(TEMP_FILE, INVENTORY_FILE) != 0) {
        printf("Error updating inventory file!\n");
        return;
    }

    if (status == 1)
        printf("Quantity successfully updated!\n");
    else if (status == 2)
        printf("Item removed from inventory!\n");
}

void listInventory(void) {
    FILE *file;
    char name[MAX_NAME];
    int quantity;
    int found = 0;
    int result;

    file = fopen(INVENTORY_FILE, "r");

    printf("=============================\n");
    printf("       Current Inventory\n");
    printf("=============================\n");

    if (file == NULL) {
        printf("The inventory is empty.\n");
        return;
    }

    while ((result = readRecord(file, name, sizeof(name),
                                &quantity)) == 1) {
        printf("Name: %s\n", name);
        printf("Quantity: %d\n\n", quantity);
        found = 1;
    }

    fclose(file);

    if (result == -1)
        printf("Error reading inventory file!\n");
    else if (!found)
        printf("The inventory is empty.\n");
}

int main(void) {
    char input[100];
    char answer;
    char *end;
    long option;

    while (1) {
        printf("\n================================\n");
        printf("       Inventory Control\n");
        printf("================================\n");
        printf("1. Add Item\n");
        printf("2. Remove Item\n");
        printf("3. List Inventory\n");
        printf("4. Exit\n");
        printf("Option: ");

        if (fgets(input, sizeof(input), stdin) == NULL)
            break;

        option = strtol(input, &end, 10);

        while (isspace((unsigned char)*end))
            end++;

        if (end == input || *end != '\0' ||
            option < 1 || option > 4) {
            printf("Invalid option! Please try again.\n");
            continue;
        }

        if (option == 4) {
            printf("Thank you for using Inventory Control! "
                   "See you next time.\n");
            break;
        }

        switch (option) {
            case 1:
                addItem();
                break;

            case 2:
                removeItem();
                break;

            case 3:
                listInventory();
                break;
        }

        printf("\nDo you want to perform another operation? (y/n): ");

        if (scanf(" %c", &answer) != 1)
            break;

        clearInput();

        if (tolower((unsigned char)answer) != 'y') {
            printf("Thank you for using Inventory Control! "
                   "See you next time.\n");
            break;
        }
    }

    return 0;
}
