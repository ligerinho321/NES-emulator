#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#define COLUMNS 64

void LoadFile(const char *fileName,unsigned char **data,long *size){

    FILE *file = fopen(fileName,"rb");

    if(!file){
        fprintf(stderr,"%s\n",strerror(errno));
        return;
    }

    fseek(file,0,SEEK_END);
    *size = ftell(file);
    fseek(file,0,SEEK_SET);

    *data = malloc(sizeof(unsigned char) * (*size));
    fread(*data,sizeof(unsigned char),*size,file);

    fclose(file);
}

void Write(FILE *head,FILE *source,const char *fileName,const char *variable){
    unsigned char *data = NULL;
    long size = 0;

    LoadFile(fileName,&data,&size);

    if(!data || !size) return;

    fprintf(head,"extern const long %s_size;\n",variable);
    fprintf(head,"extern const unsigned char %s[%d];\n\n",variable,size);

    fprintf(source,"const long %s_size = %d;\n",variable,size);
    fprintf(source,"const unsigned char %s[%d] = {\n    ",variable,size);

    for(long i=0; i<size; ++i){
        
        if(i > 0 && i % COLUMNS == 0) fprintf(source,"\n    ");

        if(i < size - 1){
            fprintf(source,"0x%.2X, ",data[i]);
        }
        else{
            fprintf(source,"0x%.2X",data[i]);
        }
    }

    fprintf(source,"\n};\n\n");

    free(data);
}

int main(int n_args,char **args){

    FILE *head = fopen("../include/Resources.h","w");

    FILE *source = fopen("../src/Resources.c","w");

    if(!head || !source){
        printf("%s\n",strerror(errno));
        return 1;
    }

    fprintf(source,"#include <Resources.h>\n\n");

    fprintf(head,"#pragma once\n\n");
    
    Write(head,source,"../assets/nespalette.pal","palette");

    Write(head,source,"../assets/arrow.png","arrow");

    Write(head,source,"../assets/circle.png","circle");

    Write(head,source,"../assets/breakpoint_arrow.png","breakpoint_arrow");

    Write(head,source,"../assets/branch1.png","branch1");

    Write(head,source,"../assets/branch2.png","branch2");

    Write(head,source,"../assets/branch3.png","branch3");
    
    Write(head,source,"../assets/checked.png","checked");

    Write(head,source,"../assets/unchecked.png","unchecked");

    Write(head,source,"../assets/help.png","help");

    Write(head,source,"../assets/play.png","play");

    Write(head,source,"../assets/pause.png","pause");

    Write(head,source,"../assets/step_into.png","step_into");

    Write(head,source,"../assets/step_over.png","step_over");

    Write(head,source,"../assets/step_out.png","step_out");

    Write(head,source,"../assets/RobotoMono-Bold.ttf","font");

    fclose(source);

    fclose(head);

    printf("resource file generated successfully.\n");
    system("pause");

    return 0;
}