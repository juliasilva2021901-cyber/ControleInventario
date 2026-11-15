
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define INVENTORY_FILE "inventory.txt"
#define TEMP_FILE "inventory.tmp"

void clearInput() {
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
        value <= 0 || value > 2147483647)
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

    name[strcspn(name, "\n")] = '\0';

    return strlen(name) > 0;
}

void addItem() {
    FILE *file;
    char name[100];
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

void removeItem() {
    FILE *file, *temp;
    char name[100];
    char storedName[100];
    int quantity, removeQuantity;
    int found = 0;
    int status = 0;
    int available = 0;

    if (!readItemName(name, sizeof(name))) {
        printf("Invalid item name!\n");
        return;
    }

    if (!readPositiveInt(
        "Enter the quantity to be removed: ",
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

    while (fgets(storedName, sizeof(storedName), file) != NULL) {
        storedName[strcspn(storedName, "\n")] = '\0';

        if (fscanf(file, "%d", &quantity) != 1) {
            fclose(file);
            fclose(temp);
            remove(TEMP_FILE);
            printf("Error reading inventory file!\n");
            return;
        }

        clearInput();

        if (!found && strcmp(name, storedName) == 0) {
            found = 1;
            available = quantity;

            if (removeQuantity > quantity) {
                status = 3;
                fprintf(temp, "%s\n%d\n", storedName, quantity);
            } else if (removeQuantity == quantity) {
                status = 2;
            } else {
                status = 1;
                fprintf(temp, "%s\n%d\n",
                        storedName, quantity - removeQuantity);
            }
        } else {
            fprintf(temp, "%s\n%d\n", storedName, quantity);
        }
    }

    fclose(file);
    fclose(temp);

    if (!found) {
        remove(TEMP_FILE);
        printf("Item not found.\n");
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

    if (status == 1) {
        printf("Quantity successfully updated!\n");
    } else if (status == 2) {
        printf("Item removed from inventory!\n");
    } else {
        printf("Insufficient stock. Available quantity: %d\n",
               available);
    }
}

void listInventory() {
    FILE *file;
    char name[100];
    int quantity;
    int found = 0;

    file = fopen(INVENTORY_FILE, "r");

    if (file == NULL) {
        printf("Inventory is empty.\n");
        return;
    }

    printf("\n========== Inventory ==========\n");

    while (fgets(name, sizeof(name), file) != NULL) {
        name[strcspn(name, "\n")] = '\0';

        if (fscanf(file, "%d", &quantity) != 1)
            break;

        clearInput();

        printf("Item: %s | Quantity: %d\n", name, quantity);
        found = 1;
    }

    if (!found)
        printf("Inventory is empty.\n");

    printf("================================\n");

    fclose(file);
}

int main() {
    int option;
    char answer;

    while (1) {
        printf("\n==================================\n");
        printf("       Inventory Control\n");
        printf("==================================\n");
        printf("1. Add Item\n");
        printf("2. Remove Item\n");
        printf("3. List Inventory\n");
        printf("4. Exit\n");
        printf("Option: ");

        char input[100];
        char *end;
        long value;

        if (fgets(input, sizeof(input), stdin) == NULL)
            break;

        value = strtol(input, &end, 10);

        while (isspace((unsigned char)*end))
            end++;

        if (end == input || *end != '\0' ||
            value < 1 || value > 4) {
            printf("Invalid option! Please try again.\n");
            continue;
        }

        option = (int)value;

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

            case 4:
                printf("Thank you for using Inventory Control! "
                       "See you next time.\n");
                return 0;
        }

        printf("\nDo you want to perform another operation? (y/n): ");

        if (scanf(" %c", &answer) != 1)
            break;

        clearInput();
        answer = (char)tolower((unsigned char)answer);

        while (answer != 'y' && answer != 'n') {
            printf("Please enter y or n: ");

            if (scanf(" %c", &answer) != 1)
                return 0;

            clearInput();
            answer = (char)tolower((unsigned char)answer);
        }

        if (answer == 'n') {
            printf("Thank you for using Inventory Control! "
                   "See you next time.\n");
            break;
        }
    }

    return 0;
}
