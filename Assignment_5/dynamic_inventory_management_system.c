#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MIN_VALUE 1
#define MAX_LENGTH 50
#define MAX_PRODUCT 100
#define MAX_PRODUCT_ID 10000
#define MIN_PRICE 0
#define MAX_PRICE 100000
#define MIN_QUANTITY 0
#define MAX_QUANTITY 1000000

typedef struct {
    int id;
    char name[MAX_LENGTH];
    float price;
    int quantity;
} ProductDetails;

void menu();
void addItem(ProductDetails **, int);
void displayProduct(ProductDetails *, int);
void displayProductData(ProductDetails *, int);
void updateQuantity(ProductDetails *, int);
void deleteProduct(ProductDetails **, int *);
void searchByID(ProductDetails *, int);
void searchByName(ProductDetails *, int);
void searchByPrice(ProductDetails *, int);
int uniqueID(ProductDetails *, int, int *);

float validPrice();
char* validName();
int validID(ProductDetails *, int);
int validQuantity();

int main(){
    printf("Enter initial number of products: ");
    int product_count;
    do{
        scanf("%d", &product_count);
        getchar(); 
        if(product_count < MIN_VALUE || product_count > MAX_PRODUCT){
            printf("\nInvalid number of products! Please enter between %d and %d: ", MIN_VALUE, MAX_PRODUCT);
        }
    } while(product_count < MIN_VALUE || product_count > MAX_PRODUCT);

    ProductDetails *products = calloc(product_count, sizeof(ProductDetails));
    for (int i = 0; i < product_count; i++) {
        printf("\nEnter details for product %d:\n", i + 1);
        addItem(&products, i);
    }

    int choice;
    do{
        menu();
        printf("\nEnter your choice: ");
        scanf("%d", &choice);
        getchar();
        switch(choice){
            case 1:
                printf("\nEnter new product details:\n");
                products = (ProductDetails *) realloc(products, (product_count + 1) * sizeof(ProductDetails));
                addItem(&products, product_count++);
                printf("\nProduct added successfully!\n");
                break;
            case 2:
                displayProduct(products, product_count);
                break;
            case 3:
                updateQuantity(products, product_count);
                break;
            case 4:
                searchByID(products, product_count);
                break;
            case 5:
                searchByName(products, product_count);
                break;
            case 6:
                searchByPrice(products, product_count);
                break;
            case 7:
                deleteProduct(&products, &product_count);
                break;
            case 8:
                free(products);
                products = NULL;
                printf("\nMemory released successfully. Exiting program...!\n");
                break;
            default:
                printf("\nEnter a valid Choice (1-8): \n");
                break;
        }
    } while(choice != 8);
    return 0;
}

void menu(){
    printf("\n========= INVENTORY MENU =========\n");
    printf("1. Add New Product.\n");
    printf("2. View All Products.\n");
    printf("3. Update Quantity.\n");
    printf("4. Search Product by ID.\n");
    printf("5. Search Product by Name.\n");
    printf("6. Search Product by Price Range.\n");
    printf("7. Delete Product.\n");
    printf("8. Exit.\n");
}

void addItem(ProductDetails **products, int count){
    (*products + count)->id = validID(*products, count);

    printf("Product Name: ");
    char *name = validName();
    strcpy((*products + count)->name, name);
    free(name);

    printf("Product Price: ");
    (*products + count)->price = validPrice();

    printf("Product Quantity: ");
    (*products + count)->quantity = validQuantity();
}

void displayProductData(ProductDetails *products, int index){
    printf("Product ID: %d | ", (products + index)->id);
    printf("Name: %s | ", (products + index)->name);
    printf("Price: %.2f | ", (products + index)->price);
    printf("Quantity: %d\n", (products + index)->quantity);
}

void displayProduct(ProductDetails *products, int count){
    if (count == 0) {
        printf("\nNo products in inventory.\n");
        return;
    }
    printf("\n========= PRODUCT LIST =========\n");
    for(int i = 0; i < count; i++){
        displayProductData(products, i);
    }
}

void updateQuantity(ProductDetails *products, int count){
    int product_id, new_quantity;
    printf("\nEnter Product ID to update quantity: ");
    scanf("%d", &product_id);
    for(int i = 0; i < count; i++){
        if(product_id == (products + i)->id){
            printf("Enter new Quantity: ");
            new_quantity = validQuantity();
            (products + i)->quantity = new_quantity;
            printf("\nQuantity updated successfully!\n");
            return;
        }
    }
    printf("\nProduct with ID %d not found.\n", product_id);
}

void deleteProduct(ProductDetails **products, int *count){
    int product_id;
    printf("\nEnter Product ID to delete: ");
    scanf("%d", &product_id);

    if(*count == 0){
        printf("\nNo products available to delete.\n");
        return;
    } else if(*count == 1 && (*products)[0].id == product_id){
        free(*products);
        *products = NULL;
        *count = 0;
        printf("\nProduct with ID %d deleted successfully.\n", product_id);
        return;
    }

    for(int i = 0; i < *count; i++){
        if(product_id == (*products + i)->id){
            for(int j = i; j < *count - 1; j++){
                *((*products) + j) = *((*products) + j + 1);
            }
            *count -= 1;
            if (*count > 0) {
                ProductDetails *temp = (ProductDetails *)realloc(*products, (*count) * sizeof(ProductDetails));
                if (temp != NULL) {
                    *products = temp;
                }
            } else {
                free(*products);
                *products = NULL;
            }
            printf("\nProduct deleted successfully!\n");
            return;
        }
    }
    printf("\nProduct with ID %d not found.\n", product_id);
}

void searchByID(ProductDetails *products, int count){
    int product_id;
    printf("\nEnter Product ID to search: ");
    scanf("%d", &product_id);

    for(int i = 0; i < count; i++){
        if(product_id == (products + i)->id){
            printf("\nProduct Found: ");
            displayProductData(products, i);
            return;
        }
    }
    printf("\nProduct with ID %d not found.\n", product_id);
}

void searchByName(ProductDetails *products, int count){
    char *name;

    printf("\nEnter name to search (partial allowed): ");
    name = validName();

    int found = 0;
    printf("\nProducts Found:\n");
    for(int i = 0; i < count; i++){
        if (strstr((products + i)->name, name) != NULL) {
            displayProductData(products, i);
            found = 1;
        }
    }

    if(!found){
        printf("\nProduct with name \"%s\" not found.\n", name);
    }

    free(name);
}

void searchByPrice(ProductDetails *products, int count){
    printf("\nEnter minimum price: ");
    float min_price = validPrice();

    printf("Enter maximum price: ");
    float max_price = validPrice();

    int found = 0;
    printf("\nProducts in price range: ");
    for(int i = 0; i < count; i++){
        if((products + i)->price <= max_price && (products + i)->price >= min_price){
            displayProductData(products, i);
            found = 1;
        }
    }

    if(!found){
        printf("(%.2f - %.2f) not found.\n", min_price, max_price);
    }
}

int uniqueID(ProductDetails *products, int count, int *id) {
    for(int i = 0; i < count; i++){
        if(*id == (products + i)->id){
            printf("\nProduct ID already exists! Enter a unique ID.\n");
            return 0;
        }
    }
    return 1;
}

int validID(ProductDetails *products, int count){
    int id;
    do {
        printf("\nProduct ID (1–%d): ", MAX_PRODUCT_ID);
        scanf("%d", &id);
        getchar();
        if (id < 1 || id > MAX_PRODUCT_ID) {
            printf("\nInvalid ID! Please enter between 1 and %d: ", MAX_PRODUCT_ID);
        }
        else if (!uniqueID(products, count, &id)) {
            id = -1; 
        }
    } while ((id < 1 || id > MAX_PRODUCT_ID));
    return id;
}

char* validName(){
    char *name = (char *)calloc(MAX_LENGTH, sizeof(char));
    fgets(name, MAX_LENGTH, stdin);
    size_t len = strlen(name);
    if (len > 0 && name[len - 1] == '\n'){
        name[len - 1] = '\0';
    }
    return name;
}

float validPrice(){
    float price;
    do {
        scanf("%f", &price);
        getchar();
        if (price < MIN_PRICE || price > MAX_PRICE) {
            printf("\nInvalid Price! Please enter between %.2f and %.2f: ", (float)MIN_PRICE, (float)MAX_PRICE);
        }
    } while (price < MIN_PRICE || price > MAX_PRICE);
    return price;
}

int validQuantity(){
    int quantity;
    do {
        scanf("%d", &quantity);
        getchar();
        if (quantity < MIN_QUANTITY || quantity > MAX_QUANTITY) {
            printf("\nInvalid Quantity! Please enter between %d and %d: ", MIN_QUANTITY, MAX_QUANTITY);
        }
    } while (quantity < MIN_QUANTITY || quantity > MAX_QUANTITY);
    return quantity;
}
