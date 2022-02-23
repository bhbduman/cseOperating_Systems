// Type your code here, or load an example.
void main(){
    char* menut= "\n1-Show Primes\n2-Factorize\n3-BubbleSort\n0-Exit\n";
    char* invalidt = "\nInvalid Number!\n";
    char* exit = "\nEXIT...\n";
    char*compt = "\nOperation Completed\n";
    int num = 0;
    while(1){
        printf("%s",menut);
        scanf("%d",&num);
        while(num<0 || num>3){
            printf("%s",invalidt);
            printf("%s",menut);
            scanf("%d",&num);
        }
        if(num==0){
            asm("li $v0, 10 \n"
            "\n\tsyscall\n");
        }
        printf("%d",num);
        asm("li $v0, 18 \n"
            "\tlw      $2,40($fp)\n"
            "\tmove $a0, $2"
            "\n\tsyscall\n");
    }
    return;



}
