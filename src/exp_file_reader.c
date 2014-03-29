/*
 * Copyright Inria:
 * Jean-François Hren
 * 
 * jfhren[at]gmail[dot]com
 * michael[dot]hauspie[at]lifl[dot]com
 * 
 * This software is a computer program whose purpose is to read, analyze,
 * modify, generate and write Java Card 2 CAP file.
 * 
 * This software is governed by the CeCILL-B license under French
 * law and
 * abiding by the rules of distribution of free software.  You can  use, 
 * modify and/ or redistribute the software under the terms of the
 * CeCILL-B
 * license as circulated by CEA, CNRS and INRIA at the following URL
 * "http://www.cecill.info". 
 * 
 * As a counterpart to the access to the source code and  rights to copy,
 * modify and redistribute granted by the license, users are provided only
 * with a limited warranty  and the software's author,  the holder of the
 * economic rights,  and the successive licensors  have only  limited
 * liability. 
 * 
 * In this respect, the user's attention is drawn to the risks associated
 * with loading,  using,  modifying and/or developing or reproducing the
 * software by the user in light of its specific status of free software,
 * that may mean  that it is complicated to manipulate,  and  that  also
 * therefore means  that it is reserved for developers  and  experienced
 * professionals having in-depth computer knowledge. Users are therefore
 * encouraged to load and test the software's suitability as regards their
 * requirements in conditions enabling the security of their systems and/or 
 * data to be ensured and,  more generally, to use and operate it in the 
 * same conditions as regards security. 
 * 
 * The fact that you are presently reading this means that you have had
 * knowledge of the CeCILL-B license and that you accept its
 * terms.
 */

/**
 * \file exp_file_reader.c
 * \brief Implement the \link read_export_file() \endlink.
 */

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>

#include "exp_file.h"

#define READ_BLOCK_SIZE 1024

static char* readFile(const char* filename, unsigned int* length) {

    char* buffer = NULL;
    int nbRead = -1;
    int alreadyRead = 0;
    int allocatedMemorySize = READ_BLOCK_SIZE;
    int fd = open(filename, O_RDONLY);

    if(fd == -1) {
        perror("readFile");
        return NULL;
    }

    buffer = (char*)malloc(sizeof(char) * READ_BLOCK_SIZE);

    if(buffer == NULL) {
        perror("readFile");
        close(fd);
        return NULL;
    }

    while((nbRead = read(fd, buffer + alreadyRead, READ_BLOCK_SIZE)) > 0) {
        alreadyRead += nbRead;
        if(alreadyRead + READ_BLOCK_SIZE > allocatedMemorySize) {
            allocatedMemorySize += READ_BLOCK_SIZE;
            char* tmp = (char*)realloc(buffer, sizeof(char) * allocatedMemorySize);
            if(tmp == NULL) {
                perror("readFile");
                free(buffer);
                close(fd);
                return NULL;
            }
            buffer = tmp;
        }
    }

    if(nbRead == -1) {
        perror("readFile");
        free(buffer);
        buffer = NULL;
    } else {
        char* tmp = realloc(buffer, sizeof(char) * alreadyRead);
        if(tmp == NULL) {
            perror("readFile");
            free(buffer);
            close(fd);
            return NULL;
        }
        buffer = tmp;
        *length = alreadyRead;
    }

    close(fd);
    return buffer;

}


static u2 bigEndianToLittleEndianU16(char* data) {

    return ((data[0] & 0xFF) << 8) | (data[1] & 0xFF);

}


static u4 bigEndianToLittleEndianU32(char* data) {

    return ((data[0] & 0xFF) << 24) | ((data[1] & 0xFF) << 16) | ((data[2] & 0xFF) << 8) | (data[3] & 0xFF);

}


static void freeConstantPool(ef_cp_info* cp, u2 count) {

    u2 i = 0;

    for(; i < count; ++i)
        if(cp[i].tag == EF_CONSTANT_PACKAGE)
            free(cp[i].CONSTANT_Package.aid);
        else if(cp[i].tag == EF_CONSTANT_UTF8)
            free(cp[i].CONSTANT_Utf8.bytes);

}


static void freeFields(ef_field_info* f, u2 count) {

    u2 i = 0;

    for(; i < count; ++i)
        free(f[i].attributes);

}


static void freeClasses(ef_class_info* c, u1 count) {

    u1 i = 0;

    for(; i < count; ++i) {
        free(c[i].supers);
        free(c[i].interfaces);
        freeFields(c[i].fields, c[i].export_fields_count);
        free(c[i].fields);
        free(c[i].methods);
    }

}


void free_export_file(export_file* ef) {

    freeConstantPool(ef->constant_pool, ef->constant_pool_count);
    free(ef->constant_pool);
    freeClasses(ef->classes, ef->export_class_count);
    free(ef->classes);

}


export_file* read_export_file(const char* filename) {

    char* data = NULL;
    export_file* ef = NULL;
    unsigned int length = 0;
    unsigned int position = 0;
    u2 indexCP = 0;
    u1 indexClass = 0;

    printf("Starting to read the export file: %s\n", filename);

    data = readFile(filename, &length);
    if(data == NULL)
        return NULL;

    if(length == 0) {
        fprintf(stderr, "No data to parse\n");
        free(data);
        return NULL;
    }

    ef = (export_file*)malloc(sizeof(export_file));
    if(ef == NULL)
        return NULL;

    if((position + 8) > length) {
        fprintf(stderr, "Not enough data to parse - %d\n", __LINE__);
        free(ef);
        free(data);
        return NULL;
    }

    ef->magic = bigEndianToLittleEndianU32(data);
    position += 4;

    ef->minor_version = data[position++];
    ef->major_version = data[position++];

    ef->constant_pool_count = bigEndianToLittleEndianU16(data + position);
    position += 2;

    if(ef->constant_pool_count < 1) {
        fprintf(stderr, "The constant pool is empty. There should be at least one element.\n");
        free(ef);
        free(data);
        return NULL;
    }

    /*printf("Constant pool count: %u\n", ef->constant_pool_count);*/

    ef->constant_pool = (ef_cp_info*)malloc(sizeof(ef_cp_info) * ef->constant_pool_count);
    if(ef->constant_pool == NULL) {
        perror("parseExportFile");
        free(ef);
        free(data);
        return NULL;
    }

    for(; indexCP < ef->constant_pool_count; ++indexCP) {
        if((position + 1) > length) {
            fprintf(stderr, "Not enough data to parse - %d\n", __LINE__);
            freeConstantPool(ef->constant_pool, indexCP);
            free(ef->constant_pool);
            free(ef);
            free(data);
            return NULL;
        }
        ef->constant_pool[indexCP].tag = data[position++];

        switch(ef->constant_pool[indexCP].tag) {
            case EF_CONSTANT_PACKAGE:
                if((position + 6) > length) {
                    fprintf(stderr, "Not enough data to parse - %d\n", __LINE__);
                    freeConstantPool(ef->constant_pool, indexCP);
                    free(ef->constant_pool);
                    free(ef);
                    free(data);
                 return NULL;
                }
                ef->constant_pool[indexCP].CONSTANT_Package.flags = data[position++];

                ef->constant_pool[indexCP].CONSTANT_Package.name_index = bigEndianToLittleEndianU16(data + position);
                position += 2;

                ef->constant_pool[indexCP].CONSTANT_Package.minor_version = data[position++];
                ef->constant_pool[indexCP].CONSTANT_Package.major_version = data[position++];

                ef->constant_pool[indexCP].CONSTANT_Package.aid_length = data[position++];

                if((position + ef->constant_pool[indexCP].CONSTANT_Package.aid_length) > length) {
                    fprintf(stderr, "Not enough data to parse - %d\n", __LINE__);
                    freeConstantPool(ef->constant_pool, indexCP);
                    free(ef->constant_pool);
                    free(ef);
                    free(data);
                 return NULL;
                }
                ef->constant_pool[indexCP].CONSTANT_Package.aid = (u1*)malloc(sizeof(u1) * ef->constant_pool[indexCP].CONSTANT_Package.aid_length);
                if(ef->constant_pool[indexCP].CONSTANT_Package.aid == NULL) {
                    perror("parseExportFile");
                    freeConstantPool(ef->constant_pool, indexCP);
                    free(ef->constant_pool);
                    free(ef);
                    free(data);
                 return NULL;
                }

                memcpy(ef->constant_pool[indexCP].CONSTANT_Package.aid, data + position, sizeof(u1) * ef->constant_pool[indexCP].CONSTANT_Package.aid_length);
                position += ef->constant_pool[indexCP].CONSTANT_Package.aid_length;

                break;

            case EF_CONSTANT_CLASSREF:
                if((position + 2) > length) {
                    fprintf(stderr, "Not enough data to parse - %d\n", __LINE__);
                    freeConstantPool(ef->constant_pool, indexCP);
                    free(ef->constant_pool);
                    free(ef);
                    free(data);
                 return NULL;
                }
                ef->constant_pool[indexCP].CONSTANT_Classref.name_index = bigEndianToLittleEndianU16(data + position);
                position += 2;

                break;

            case EF_CONSTANT_INTEGER:
                if((position + 4) > length) {
                    fprintf(stderr, "Not enough data to parse - %d\n", __LINE__);
                    freeConstantPool(ef->constant_pool, indexCP);
                    free(ef->constant_pool);
                    free(ef);
                    free(data);
                 return NULL;
                }
                ef->constant_pool[indexCP].CONSTANT_Integer.bytes = bigEndianToLittleEndianU32(data + position);
                position += 4;

                break;

            case EF_CONSTANT_UTF8:
                if((position + 2) > length) {
                    fprintf(stderr, "Not enough data to parse - %d\n", __LINE__);
                    freeConstantPool(ef->constant_pool, indexCP);
                    free(ef->constant_pool);
                    free(ef);
                    free(data);
                 return NULL;
                }
                ef->constant_pool[indexCP].CONSTANT_Utf8.length = bigEndianToLittleEndianU16(data + position);
                position += 2;

                if((position + ef->constant_pool[indexCP].CONSTANT_Utf8.length) > length) {
                    fprintf(stderr, "Not enough data to parse - %d\n", __LINE__);
                    freeConstantPool(ef->constant_pool, indexCP);
                    free(ef->constant_pool);
                    free(ef);
                    free(data);
                 return NULL;
                }
                ef->constant_pool[indexCP].CONSTANT_Utf8.bytes = (u1*)malloc(sizeof(u1) * ef->constant_pool[indexCP].CONSTANT_Utf8.length);
                if(ef->constant_pool[indexCP].CONSTANT_Utf8.bytes == NULL) {
                    perror("parseExportFile");
                    freeConstantPool(ef->constant_pool, indexCP);
                    free(ef->constant_pool);
                    free(ef);
                    free(data);
                    return NULL;
                }

                memcpy(ef->constant_pool[indexCP].CONSTANT_Utf8.bytes, data + position, sizeof(u1) * ef->constant_pool[indexCP].CONSTANT_Utf8.length);
                position += ef->constant_pool[indexCP].CONSTANT_Utf8.length;

                /*printf("\tIndex %u: %.*s\n", indexCP, ef->constant_pool[indexCP].CONSTANT_Utf8.length, ef->constant_pool[indexCP].CONSTANT_Utf8.bytes);*/

                break;

                default:
                    fprintf(stderr, "The tag %u is not supported.\n", ef->constant_pool[indexCP].tag);
                    freeConstantPool(ef->constant_pool, indexCP);
                    free(ef->constant_pool);
                    free(ef);
                    free(data);
                    return NULL;
        }
    }

    if((position + 3) > length) {
        fprintf(stderr, "Not enough data to parse - %d\n", __LINE__);
        freeConstantPool(ef->constant_pool, ef->constant_pool_count);
        free(ef->constant_pool);
        free(ef);
        free(data);
        return NULL;
    }
    ef->this_package = bigEndianToLittleEndianU16(data + position);
    position += 2;

    ef->export_class_count = data[position++];

    /*printf("class count: %u\n", ef->export_class_count);*/

    if(ef->export_class_count == 0) {
        ef->classes = NULL;
        return ef;
    }

    ef->classes = (ef_class_info*)malloc(sizeof(ef_class_info) * ef->export_class_count);
    if(ef->classes == NULL) {
        perror("parseExportFile");
        freeConstantPool(ef->constant_pool, ef->constant_pool_count);
        free(ef->constant_pool);
        free(ef);
        free(data);
        return NULL;
    }

    for(; indexClass < ef->export_class_count; ++indexClass) {
        u2 index = 0;

        if((position + 7) > length) {
            fprintf(stderr, "Not enough data to parse - %d\n", __LINE__);
            freeConstantPool(ef->constant_pool, ef->constant_pool_count);
            free(ef->constant_pool);
            freeClasses(ef->classes, indexClass);
            free(ef->classes);
            free(ef);
            free(data);
            return NULL;
        }
        ef->classes[indexClass].token = data[position++];

        ef->classes[indexClass].access_flags = bigEndianToLittleEndianU16(data + position);
        position += 2;

        ef->classes[indexClass].name_index = bigEndianToLittleEndianU16(data + position);
        position += 2;

        ef->classes[indexClass].export_supers_count = bigEndianToLittleEndianU16(data + position);
        position += 2;

        /*printf("Class %u\n\tSupers count:%u\n", indexClass, ef->classes[indexClass].export_supers_count);*/

        if(ef->classes[indexClass].export_supers_count > 0) {

            if((position + (2 * ef->classes[indexClass].export_supers_count) + 1) > length) {
                fprintf(stderr, "Not enough data to parse - %d\n", __LINE__);
                freeConstantPool(ef->constant_pool, ef->constant_pool_count);
                free(ef->constant_pool);
                freeClasses(ef->classes, indexClass);
                free(ef->classes);
                free(ef);
                free(data);
                return NULL;
            }
            ef->classes[indexClass].supers = (u2*)malloc(sizeof(u2) * ef->classes[indexClass].export_supers_count);
            if(ef->classes[indexClass].supers == NULL) {
                perror("parseExportFile");
                freeConstantPool(ef->constant_pool, ef->constant_pool_count);
                free(ef->constant_pool);
                freeClasses(ef->classes, indexClass);
                free(ef->classes);
                free(ef);
                free(data);
                return NULL;
            }

            for(; index < ef->classes[indexClass].export_supers_count; ++index) {
                ef->classes[indexClass].supers[index] = bigEndianToLittleEndianU16(data + position);
                position += 2;
            }

        } else {
            if((position + 1) > length) {
                fprintf(stderr, "Not enough data to parse - %d\n", __LINE__);
                freeConstantPool(ef->constant_pool, ef->constant_pool_count);
                free(ef->constant_pool);
                freeClasses(ef->classes, indexClass);
                free(ef->classes);
                free(ef);
                free(data);
                return NULL;
            }
            ef->classes[indexClass].supers = NULL;
        }

        ef->classes[indexClass].export_interfaces_count = data[position++];

       /*printf("\tInterfaces count: %u\n", ef->classes[indexClass].export_interfaces_count);*/

        if(ef->classes[indexClass].export_interfaces_count > 0) {

            if((position + (2 * ef->classes[indexClass].export_interfaces_count) + 2) > length) {
                fprintf(stderr, "Not enough data to parse - %d\n", __LINE__);
                freeConstantPool(ef->constant_pool, ef->constant_pool_count);
                free(ef->constant_pool);
                free(ef->classes[indexClass].supers);
                freeClasses(ef->classes, indexClass);
                free(ef->classes);
                free(ef);
                free(data);
                return NULL;
            }
            ef->classes[indexClass].interfaces = (u2*)malloc(sizeof(u2) * ef->classes[indexClass].export_interfaces_count);
            if(ef->classes[indexClass].interfaces == NULL) {
                perror("parseExportFile");
                freeConstantPool(ef->constant_pool, ef->constant_pool_count);
                free(ef->constant_pool);
                free(ef->classes[indexClass].supers);
                freeClasses(ef->classes, indexClass);
                free(ef->classes);
                free(ef);
                free(data);
                return NULL;
            }

            for(index = 0; index < ef->classes[indexClass].export_interfaces_count; ++index) {
                ef->classes[indexClass].interfaces[index] = bigEndianToLittleEndianU16(data + position);
                position += 2;
            }

        } else {
            ef->classes[indexClass].interfaces = NULL;
        }

        ef->classes[indexClass].export_fields_count = bigEndianToLittleEndianU16(data + position);
        position += 2;

        /*printf("\tFields count: %u\n", ef->classes[indexClass].export_fields_count);*/

        if(ef->classes[indexClass].export_fields_count > 0) {

            ef->classes[indexClass].fields = (ef_field_info*)malloc(sizeof(ef_field_info) * ef->classes[indexClass].export_fields_count);
            if(ef->classes[indexClass].fields == NULL) {
                perror("parseExportFile");
                freeConstantPool(ef->constant_pool, ef->constant_pool_count);
                free(ef->constant_pool);
                free(ef->classes[indexClass].supers);
                free(ef->classes[indexClass].interfaces);
                freeClasses(ef->classes, indexClass);
                free(ef->classes);
                free(ef);
                free(data);
                return NULL;
            }

            for(index = 0; index < ef->classes[indexClass].export_fields_count; ++index) {
                u2 indexAttribute = 0;
                if((position + 9) > length) {
                    fprintf(stderr, "Not enough data to parse - %d\n", __LINE__);
                    freeConstantPool(ef->constant_pool, ef->constant_pool_count);
                    free(ef->constant_pool);
                    free(ef->classes[indexClass].supers);
                    free(ef->classes[indexClass].interfaces);
                    freeFields(ef->classes[indexClass].fields, index);
                    free(ef->classes[indexClass].fields);
                    freeClasses(ef->classes, indexClass);
                    free(ef->classes);
                    free(ef);
                    free(data);
                    return NULL;
                }
                ef->classes[indexClass].fields[index].token = data[position++];

                ef->classes[indexClass].fields[index].access_flags = bigEndianToLittleEndianU16(data + position);
                position += 2;

                ef->classes[indexClass].fields[index].name_index = bigEndianToLittleEndianU16(data + position);
                position += 2;

                ef->classes[indexClass].fields[index].descriptor_index = bigEndianToLittleEndianU16(data + position);
                position += 2;

                ef->classes[indexClass].fields[index].attributes_count = bigEndianToLittleEndianU16(data + position);
                position += 2;

                /*printf("\tField %u\n\t\tAttributes count: %u\n", index, ef->classes[indexClass].fields[index].attributes_count);*/

                if(ef->classes[indexClass].fields[index].attributes_count > 0) {

                    if((position + (8 * ef->classes[indexClass].fields[index].attributes_count) + 2) > length) {
                        fprintf(stderr, "Not enough data to parse - %d\n", __LINE__);
                        freeConstantPool(ef->constant_pool, ef->constant_pool_count);
                        free(ef->constant_pool);
                        free(ef->classes[indexClass].supers);
                        free(ef->classes[indexClass].interfaces);
                        freeFields(ef->classes[indexClass].fields, index);
                        free(ef->classes[indexClass].fields);
                        freeClasses(ef->classes, indexClass);
                        free(ef->classes);
                        free(ef);
                        free(data);
                        return NULL;
                    }
                    ef->classes[indexClass].fields[index].attributes = (ef_attribute_info*)malloc(sizeof(ef_attribute_info) * ef->classes[indexClass].fields[index].attributes_count);
                    if(ef->classes[indexClass].fields[index].attributes == NULL) {
                        perror("parseExportFile");
                        freeConstantPool(ef->constant_pool, ef->constant_pool_count);
                        free(ef->constant_pool);
                        free(ef->classes[indexClass].supers);
                        free(ef->classes[indexClass].interfaces);
                        freeFields(ef->classes[indexClass].fields, index);
                        free(ef->classes[indexClass].fields);
                        freeClasses(ef->classes, indexClass);
                        free(ef->classes);
                        free(ef);
                        free(data);
                        return NULL;
                    }

                    for(; indexAttribute < ef->classes[indexClass].fields[index].attributes_count; ++indexAttribute) {
                        ef->classes[indexClass].fields[index].attributes[indexAttribute].attribute_name_index = bigEndianToLittleEndianU16(data + position);
                        position += 2;

                        ef->classes[indexClass].fields[index].attributes[indexAttribute].attribute_length = bigEndianToLittleEndianU32(data + position);
                        position += 4;

                        ef->classes[indexClass].fields[index].attributes[indexAttribute].constantvalue_index = bigEndianToLittleEndianU16(data + position);
                        position += 2;
                    }

                } else {
                    ef->classes[indexClass].fields[index].attributes = NULL;
                }
            }

        } else {
            ef->classes[indexClass].fields = NULL;
        }

        ef->classes[indexClass].export_methods_count = bigEndianToLittleEndianU16(data + position);
        position += 2;

        /*printf("\tMethods count: %u\n",ef->classes[indexClass].export_methods_count);*/

        if(ef->classes[indexClass].export_methods_count > 0) {

            if((position + (7 * ef->classes[indexClass].export_methods_count)) > length) {
                fprintf(stderr, "Not enough data to parse - %d\n", __LINE__);
                freeConstantPool(ef->constant_pool, ef->constant_pool_count);
                free(ef->constant_pool);
                free(ef->classes[indexClass].supers);
                free(ef->classes[indexClass].interfaces);
                freeFields(ef->classes[indexClass].fields, ef->classes[indexClass].export_fields_count);
                free(ef->classes[indexClass].fields);
                freeClasses(ef->classes, indexClass);
                free(ef->classes);
                free(ef);
                free(data);
                return NULL;
            }
            ef->classes[indexClass].methods = (ef_method_info*)malloc(sizeof(ef_method_info) * ef->classes[indexClass].export_methods_count);
            if(ef->classes[indexClass].methods == NULL) {
                perror("parseExportFile");
                freeConstantPool(ef->constant_pool, ef->constant_pool_count);
                free(ef->constant_pool);
                free(ef->classes[indexClass].supers);
                free(ef->classes[indexClass].interfaces);
                freeFields(ef->classes[indexClass].fields, ef->classes[indexClass].export_fields_count);
                free(ef->classes[indexClass].fields);
                freeClasses(ef->classes, indexClass);
                free(ef->classes);
                free(ef);
                free(data);
                return NULL;
            }

            for(index = 0; index < ef->classes[indexClass].export_methods_count; ++index) {
                ef->classes[indexClass].methods[index].token = data[position++];

                ef->classes[indexClass].methods[index].access_flags = bigEndianToLittleEndianU16(data + position);
                position += 2;

                ef->classes[indexClass].methods[index].name_index = bigEndianToLittleEndianU16(data + position);
                position += 2;

                ef->classes[indexClass].methods[index].descriptor_index = bigEndianToLittleEndianU16(data + position);
                position += 2;
            }

        } else {
            ef->classes[indexClass].methods = NULL;
        }

    }

    return ef;

}
