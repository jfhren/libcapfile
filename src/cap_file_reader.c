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
 * \file cap_file_reader.c
 * \brief Implement the ::read_cap_file function.
 */

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <zip.h>

#include "cap_file.h"

#define READ_BLOCK_SIZE 1024

/**
 * \brief Read a component in the zipped cap file.
 *
 * Cap files are zipped with each component being a file within the zip. This
 * function extracts the component content into an allocated buffer and returns
 * it.
 *
 * \param z        The cap file opened as a zip file.
 * \param filename The name of the component to read.
 *
 * \return An allocated buffer with the read component in it.
 */
static char* readZipFile(struct zip* z, const char* filename) {

    char* buffer = NULL;
    int nbRead = -1;
    int alreadyRead = 0;
    int allocatedMemorySize = READ_BLOCK_SIZE;
    struct zip_file* zf = zip_fopen(z, filename, 0);

    if(zf == NULL) {
        fprintf(stderr, "%s\n", zip_strerror(z));
        return NULL;
    }

    buffer = (char*)malloc(sizeof(char) * READ_BLOCK_SIZE);

    if(buffer == NULL) {
        perror("readZipFile");
        zip_fclose(zf);
        return NULL;
    }

    while((nbRead = zip_fread(zf, buffer + alreadyRead, READ_BLOCK_SIZE)) > 0) {
        alreadyRead += nbRead;
        if(alreadyRead + READ_BLOCK_SIZE > allocatedMemorySize) {
            allocatedMemorySize += READ_BLOCK_SIZE;
            char* tmp = (char*)realloc(buffer, sizeof(char) * allocatedMemorySize);
            if(tmp == NULL) {
                perror("readZipFile");
                free(buffer);
                zip_fclose(zf);
                return NULL;
            }
            buffer = tmp;
        }
    }

    if(nbRead == -1) {
        fprintf(stderr, "%s\n", zip_strerror(z));
        free(buffer);
        buffer = NULL;
    } else {
        char* tmp = realloc(buffer, sizeof(char) * alreadyRead);
        if(tmp == NULL) {
            perror("readFile");
            free(buffer);
            zip_fclose(zf);
            return NULL;
        }
        buffer = tmp;
    }

    zip_fclose(zf);
    return buffer;

}


/**
 * \brief Take two bytes in big endian order and make an unsigned 16 bits
 *        integer.
 *
 * \param data The array from which the bytes are taken.
 *
 * \return The built unsigned 16-bits integer.
 */
static u2 bigEndianToU2(char* data) {

    return ((data[0] & 0xFF) << 8) | (data[1] & 0xFF);

}


/**
 * \brief Take four bytes in big endian order and make an unsigned 32 bits
 *        integer.
 *
 * \param data The array from which the bytes are taken.
 *
 * \return The built unsigned 16-bits integer.
 */
static u4 bigEndianToU4(char* data) {

    return ((data[0] & 0xFF) << 24) | ((data[1] & 0xFF) << 16) | ((data[2] & 0xFF) << 8) | (data[3] & 0xFF);

}


/** 
 * \brief Parse the header component.
 *
 * The function parses the header component (see 6.3 Header Component of Virtual
 * Machine Specification, Java Card Platform, v2.2.2) from the input data and
 * output it straightforwardly into the header field of a cap_file struct.
 *
 * \param data The input data containing the raw header component.
 * \param cf   The structured representation of a cap file with a completed
 *             header component field.
 *
 * \return Return -1 if an error occured, 0 else.
 */
static int parseHeaderComponent(cap_file* cf, char* data) {

    unsigned int position = 0;

    if(data[position] != COMPONENT_HEADER) {
        fprintf(stderr, "It should be a header component\n");
        return -1;
    }

    printf("Parsing header component\n");
    cf->header.tag = data[position++];

    cf->header.size = bigEndianToU2(data + position);
    position += 2;

    cf->header.magic = bigEndianToU4(data + position);
    position += 4;

    cf->header.minor_version = data[position++];
    cf->header.major_version = data[position++];
    cf->header.flags = data[position++];

    cf->header.package.minor_version = data[position++];
    cf->header.package.major_version = data[position++];

    cf->header.package.AID_length = data[position++];
    cf->header.package.AID = (u1*)malloc(sizeof(u1) * cf->header.package.AID_length);
    if(cf->header.package.AID == NULL) {
        perror("parseHeaderComponent");
        return -1;
    }
    memcpy(cf->header.package.AID, data + position, sizeof(u1) * cf->header.package.AID_length);
    position += cf->header.package.AID_length;

    if((cf->header.major_version == 2) && (cf->header.minor_version > 1)) {
        cf->header.has_package_name = 1;
        cf->header.package_name.name_length = data[position++];
        cf->header.package_name.name = (u1*)malloc(sizeof(u1) * cf->header.package_name.name_length);
        if(cf->header.package_name.name == NULL) {
            perror("parseHeaderComponent");
            return -1;
        }
        memcpy(cf->header.package_name.name, data + position, sizeof(u1) * cf->header.package_name.name_length);
        position += cf->header.package_name.name_length;
    } else {
        cf->header.has_package_name = 0;
        cf->header.package_name.name_length = 0;
        cf->header.package_name.name = NULL;
    }

    if(position != (cf->header.size + 3u)) {
        fprintf(stderr, "Parsing incomplete\n");
        return -1;
    }

    return 0;

}


/**
 * \brief Parse the directory component.
 *
 * The function parses the directory component (see 6.4 Directory Component of
 * Virtual Machine Specification, Java Card Platform, v2.2.2) from the input
 * data and output it straightforwardly into the directory field of a cap_file
 * struct. The header component should be read before calling this function.
 *
 * \param data The input data containing the raw directory component.
 * \param cf   The structured representation of a cap file with a completed
 *             directory component field.
 *
 * \return Return -1 if an error occured, 0 else.
 */
static int parseDirectoryComponent(cap_file* cf, char* data) {

    unsigned int position = 0;
    u1 u1Index = 0;

    if(data[position] != COMPONENT_DIRECTORY) {
        fprintf(stderr, "It should be a directory component: %x\n", data[position]);
        return -1;
    }

    printf("Parsing directory component\n");
    cf->directory.tag = data[position++];
    cf->directory.size = bigEndianToU2(data + position);
    position += 2;

    for(u1Index = 0; u1Index < 11; ++u1Index) {
        cf->directory.component_sizes[u1Index] = bigEndianToU2(data + position);
        position += 2;
    }

    if((cf->header.major_version == 2) && (cf->header.minor_version > 1)) {
        cf->directory.can_have_debug_component = 1;
        cf->directory.component_sizes[11] = bigEndianToU2(data + position);
        position += 2;
    } else {
        cf->directory.can_have_debug_component = 0;
        cf->directory.component_sizes[11] = 0;
    }

    cf->directory.static_field_size.image_size = bigEndianToU2(data + position);
    position += 2;
    cf->directory.static_field_size.array_init_count = bigEndianToU2(data + position);
    position += 2;
    cf->directory.static_field_size.array_init_size = bigEndianToU2(data + position);
    position += 2;

    cf->directory.import_count = data[position++];
    cf->directory.applet_count = data[position++];
    cf->directory.custom_count = data[position++];

    if(cf->directory.custom_count == 0) {
        cf->directory.custom_components = NULL;
        return 0;
    }

    cf->directory.custom_components = (cf_custom_component_info*)malloc(sizeof(cf_custom_component_info) * cf->directory.custom_count);
    if(cf->directory.custom_components == NULL) {
        perror("parseDirectoryComponent");
        return -1;
    }
    for(u1Index = 0; u1Index < cf->directory.custom_count; ++u1Index) {
        cf->directory.custom_components[u1Index].component_tag = data[position++];
        cf->directory.custom_components[u1Index].size = bigEndianToU2(data + position);
        position += 2;
        cf->directory.custom_components[u1Index].AID_length = data[position++]; 
        cf->directory.custom_components[u1Index].AID = (u1*)malloc(sizeof(u1) * cf->directory.custom_components[u1Index].AID_length);
        if(cf->directory.custom_components[u1Index].AID == NULL) {
            perror("parseDirectoryComponent");
            return -1;
        }
        memcpy(cf->directory.custom_components[u1Index].AID, data + position, sizeof(u1) * cf->directory.custom_components[u1Index].AID_length);
        position += cf->directory.custom_components[u1Index].AID_length;
    }

    if(position != (cf->directory.size + 3u)) {
        fprintf(stderr, "Parsing incomplete\n");
        return -1;
    }

    return 0;

}


/**
 * \brief Parse the applet component.
 *
 * The function parses the applet component (see 6.5 Applet Component of Virtual
 * Machine Specification, Java Card Platform, v2.2.2) from the input data and
 * output it straightforwardly into the applet field of a cap_file struct.
 *
 * \param data The input data containing the raw applet component.
 * \param cf   The structured representation of a cap file with a completed
 *             applet component field.
 *
 * \return Return -1 if an error occured, 0 else.
 */
static int parseAppletComponent(cap_file* cf, char* data) {

    unsigned int position = 0;
    u1 u1Index = 0;

    if(data[position] != COMPONENT_APPLET) {
        fprintf(stderr, "It should be an applet component\n");
        return -1;
    }

    printf("Parsing applet component\n");
    cf->applet.tag = data[position++];
    cf->applet.size = bigEndianToU2(data + position);
    position += 2;

    cf->applet.count = data[position++];
    cf->applet.applets = (cf_applet_info*)malloc(sizeof(cf_applet_info) * cf->applet.count);
    if(cf->applet.applets == NULL) {
        perror("parseAppletComponent");
        return -1;
    }

    for(u1Index = 0; u1Index < cf->applet.count; ++u1Index) {
        cf->applet.applets[u1Index].AID_length = data[position++];
        cf->applet.applets[u1Index].AID = (u1*)malloc(sizeof(u1) * cf->applet.applets[u1Index].AID_length);
        if(cf->applet.applets[u1Index].AID == NULL) {
            perror("parseAppletComponent");
            return -1;
        }
        memcpy(cf->applet.applets[u1Index].AID, data + position, sizeof(u1) * cf->applet.applets[u1Index].AID_length);
        position += cf->applet.applets[u1Index].AID_length;
        cf->applet.applets[u1Index].install_method_offset = bigEndianToU2(data + position);
        position += 2;
    }

    if(position != (cf->applet.size + 3u)) {
        fprintf(stderr, "Parsing incomplete\n");
        return -1;
    }

    return 0;

}


/**
 * \brief Parse the import component.
 *
 * The function parses the import component (see 6.6 Import Component of Virtual
 * Machine Specification, Java Card Platform, v2.2.2) from the input data and
 * output it straightforwardly into the import field of a cap_file struct.
 *
 * \param data The input data containing the raw import component.
 * \param cf   The structured representation of a cap file with a completed
 *             import component field.
 *
 * \return Return -1 if an error occured, 0 else.
 */
static int parseImportComponent(cap_file* cf, char* data) {

    unsigned int position = 0;
    u1 u1Index = 0;

    if(data[position] != COMPONENT_IMPORT) {
        fprintf(stderr, "It should be an import component\n");
        return -1;
    }

    printf("Parsing import component\n");
    cf->import.tag = data[position++];
    cf->import.size = bigEndianToU2(data + position);
    position += 2;

    cf->import.count = data[position++];
    cf->import.packages = (cf_package_info*)malloc(sizeof(cf_package_info) * cf->import.count);
    if(cf->import.packages == NULL) {
        perror("parseImportComponent");
        return -1;
    }

    for(u1Index = 0; u1Index < cf->import.count; ++u1Index) {
        cf->import.packages[u1Index].minor_version = data[position++];
        cf->import.packages[u1Index].major_version = data[position++];
        cf->import.packages[u1Index].AID_length = data[position++];
        cf->import.packages[u1Index].AID = (u1*)malloc(sizeof(u1) * cf->import.packages[u1Index].AID_length);
        if(cf->import.packages[u1Index].AID == NULL) {
            perror("parseImportComponent");
            return -1;
        }
        memcpy(cf->import.packages[u1Index].AID, data + position, sizeof(u1) * cf->import.packages[u1Index].AID_length);
        position += cf->import.packages[u1Index].AID_length;
    }

    if(position != (cf->import.size + 3u)) {
        fprintf(stderr, "Parsing incomplete\n");
        return -1;
    }

    return 0;

}


/**
 * \brief Parse the constant pool component.
 *
 * The function parses the constant pool component (see 6.7 Constant Pool
 * Component of Virtual Machine Specification, Java Card Platform, v2.2.2) from
 * the input data and output it straightforwardly into the constant_pool field
 * of a cap_file struct.
 *
 * \param data The input data containing the raw constant pool component.
 * \param cf The structured representation of a cap file with a completed
 *           constant pool component field.
 *
 * \return Return -1 if an error occured, 0 else.
 */
static int parseConstantPoolComponent(cap_file* cf, char* data) {

    unsigned int position = 0;
    u2 u2Index = 0;

    if(data[position] != COMPONENT_CONSTANTPOOL) {
        fprintf(stderr, "It should be a constant pool component\n");
        return -1;
    }

    printf("Parsing constant pool component\n");
    cf->constant_pool.tag = data[position++];
    cf->constant_pool.size = bigEndianToU2(data + position);
    position += 2;

    cf->constant_pool.count = bigEndianToU2(data + position);
    position += 2;

    cf->constant_pool.constant_pool = (cf_cp_info*)malloc(sizeof(cf_cp_info) * cf->constant_pool.count);
    if(cf->constant_pool.constant_pool == NULL) {
        perror("parseConstantPoolComponent");
        return -1;
    }

    for(u2Index = 0; u2Index < cf->constant_pool.count; ++u2Index) {
        cf->constant_pool.constant_pool[u2Index].tag = data[position++];

        switch(cf->constant_pool.constant_pool[u2Index].tag) {
            case CF_CONSTANT_CLASSREF:
                cf->constant_pool.constant_pool[u2Index].CONSTANT_Classref.class_ref.isExternal = (data[position] & 0x80) ? 1 : 0;
                if(cf->constant_pool.constant_pool[u2Index].CONSTANT_Classref.class_ref.isExternal) {
                    cf->constant_pool.constant_pool[u2Index].CONSTANT_Classref.class_ref.ref.external_class_ref.package_token = data[position++] & 0x7F;
                    cf->constant_pool.constant_pool[u2Index].CONSTANT_Classref.class_ref.ref.external_class_ref.class_token = data[position++];
                } else {
                    cf->constant_pool.constant_pool[u2Index].CONSTANT_Classref.class_ref.ref.internal_class_ref = bigEndianToU2(data + position);
                    position += 2;
                }
                cf->constant_pool.constant_pool[u2Index].CONSTANT_Classref.padding = data[position++];
                break;

            case CF_CONSTANT_INSTANCEFIELDREF:
                cf->constant_pool.constant_pool[u2Index].CONSTANT_InstanceFieldref.class.isExternal = (data[position] & 0x80) ? 1 : 0;
                if(cf->constant_pool.constant_pool[u2Index].CONSTANT_InstanceFieldref.class.isExternal) {
                    cf->constant_pool.constant_pool[u2Index].CONSTANT_InstanceFieldref.class.ref.external_class_ref.package_token = data[position++] & 0x7F;
                    cf->constant_pool.constant_pool[u2Index].CONSTANT_InstanceFieldref.class.ref.external_class_ref.class_token = data[position++];
                } else {
                    cf->constant_pool.constant_pool[u2Index].CONSTANT_InstanceFieldref.class.ref.internal_class_ref = bigEndianToU2(data + position);
                    position += 2;
                }
                cf->constant_pool.constant_pool[u2Index].CONSTANT_InstanceFieldref.token = data[position++];
                break;

            case CF_CONSTANT_VIRTUALMETHODREF:
                cf->constant_pool.constant_pool[u2Index].CONSTANT_VirtualMethodref.class.isExternal = (data[position] & 0x80) ? 1 : 0;
                if(cf->constant_pool.constant_pool[u2Index].CONSTANT_VirtualMethodref.class.isExternal) {
                    cf->constant_pool.constant_pool[u2Index].CONSTANT_VirtualMethodref.class.ref.external_class_ref.package_token = data[position++] & 0x7F;
                    cf->constant_pool.constant_pool[u2Index].CONSTANT_VirtualMethodref.class.ref.external_class_ref.class_token = data[position++];
                } else {
                    cf->constant_pool.constant_pool[u2Index].CONSTANT_VirtualMethodref.class.ref.internal_class_ref = bigEndianToU2(data + position);
                    position += 2;
                }
                cf->constant_pool.constant_pool[u2Index].CONSTANT_VirtualMethodref.token = data[position++];
                break;

            case CF_CONSTANT_SUPERMETHODREF:
                /* In practice, external class reference is not possible for super method reference. */
                cf->constant_pool.constant_pool[u2Index].CONSTANT_SuperMethodref.class.isExternal = (data[position] & 0x80) ? 1 : 0;
                if(cf->constant_pool.constant_pool[u2Index].CONSTANT_SuperMethodref.class.isExternal) {
                    cf->constant_pool.constant_pool[u2Index].CONSTANT_SuperMethodref.class.ref.external_class_ref.package_token = data[position++] & 0x7F;
                    cf->constant_pool.constant_pool[u2Index].CONSTANT_SuperMethodref.class.ref.external_class_ref.class_token = data[position++];
                } else {
                    cf->constant_pool.constant_pool[u2Index].CONSTANT_SuperMethodref.class.ref.internal_class_ref = bigEndianToU2(data + position);
                    position += 2;
                }
                cf->constant_pool.constant_pool[u2Index].CONSTANT_SuperMethodref.token = data[position++];
                break;

            case CF_CONSTANT_STATICFIELDREF:
                cf->constant_pool.constant_pool[u2Index].CONSTANT_StaticFieldref.static_field_ref.isExternal = (data[position] & 0x80) ? 1 : 0;
                if(cf->constant_pool.constant_pool[u2Index].CONSTANT_StaticFieldref.static_field_ref.isExternal) {
                    cf->constant_pool.constant_pool[u2Index].CONSTANT_StaticFieldref.static_field_ref.ref.external_ref.package_token = data[position++] & 0x7F;
                    cf->constant_pool.constant_pool[u2Index].CONSTANT_StaticFieldref.static_field_ref.ref.external_ref.class_token = data[position++];
                    cf->constant_pool.constant_pool[u2Index].CONSTANT_StaticFieldref.static_field_ref.ref.external_ref.token = data[position++];
                } else {
                    cf->constant_pool.constant_pool[u2Index].CONSTANT_StaticFieldref.static_field_ref.ref.internal_ref.padding = data[position++];
                    cf->constant_pool.constant_pool[u2Index].CONSTANT_StaticFieldref.static_field_ref.ref.internal_ref.offset = bigEndianToU2(data + position);
                    position += 2;
                }
                break;

            case CF_CONSTANT_STATICMETHODREF:
                cf->constant_pool.constant_pool[u2Index].CONSTANT_StaticMethodref.static_method_ref.isExternal = (data[position] & 0x80) ? 1 : 0;
                if(cf->constant_pool.constant_pool[u2Index].CONSTANT_StaticMethodref.static_method_ref.isExternal) {
                    cf->constant_pool.constant_pool[u2Index].CONSTANT_StaticMethodref.static_method_ref.ref.external_ref.package_token = data[position++] & 0x7F;
                    cf->constant_pool.constant_pool[u2Index].CONSTANT_StaticMethodref.static_method_ref.ref.external_ref.class_token = data[position++];
                    cf->constant_pool.constant_pool[u2Index].CONSTANT_StaticMethodref.static_method_ref.ref.external_ref.token = data[position++];
                } else {
                    cf->constant_pool.constant_pool[u2Index].CONSTANT_StaticMethodref.static_method_ref.ref.internal_ref.padding = data[position++];
                    cf->constant_pool.constant_pool[u2Index].CONSTANT_StaticMethodref.static_method_ref.ref.internal_ref.offset = bigEndianToU2(data + position);
                    position += 2;
                }
                break;

            default:
                fprintf(stderr, "Constant pool tag %u not supported\n", cf->constant_pool.constant_pool[u2Index].tag);
                return -1;
        }
    }

    if(position != (cf->constant_pool.size + 3u)) {
        fprintf(stderr, "Constant pool: parsing incomplete (%u != %u)\n", position, cf->constant_pool.size + 3u);
        return -1;
    }

    return 0;

}


/**
 * \brief Parse the import component.
 *
 * The function parses the class component (see 6.8 Class Component of Virtual
 * Machine Specification, Java Card Platform, v2.2.2) from the input data and
 * output it straightforwardly into the class field of a cap_file struct. The
 * header component should be read before calling this function.
 *
 * \param data The input data containing the raw class component.
 * \param cf   The structured representation of a cap file with a completed
 *             class component field.
 *
 * \return Return -1 if an error occured, 0 else.
 */
static int parseClassComponent(cap_file* cf, char* data) {

    unsigned int position = 0;
    unsigned initialPosition = 0;
    u2 crtSize = 0;
    u2 size = 0;
    u1 u1Index = 0;
    u2 offset = 0;

    if(data[position] != COMPONENT_CLASS) {
        fprintf(stderr, "It should be a class component\n");
        return -1;
    }

    printf("Parsing class component\n");
    cf->class.tag = data[position++];
    cf->class.size = bigEndianToU2(data + position);
    position += 2;
    initialPosition = position;
    size = cf->class.size;

    if((cf->header.major_version == 2) && (cf->header.minor_version > 1)) {
        cf->class.can_have_signature_pool = 1;
        u2 usedLength = 0;
        u2 u2Index = 0;
        cf->class.signature_pool_length = bigEndianToU2(data + position);
        position += 2;
        cf->class.signature_pool_count = 0;
        cf->class.signature_pool = NULL;

        while(usedLength < cf->class.signature_pool_length) {
            u1 array_type_count = 0;
            cf_type_descriptor* tmp = (cf_type_descriptor*)realloc(cf->class.signature_pool, sizeof(cf_type_descriptor) * (u2Index + 1));
            if(tmp == NULL) {
                perror("parseClassComponent");
                return -1;
            }
            cf->class.signature_pool = tmp;
            ++cf->class.signature_pool_length;

            cf->class.signature_pool[u2Index].offset = offset;
            cf->class.signature_pool[u2Index].nibble_count = data[position++];
            array_type_count = (cf->class.signature_pool[u2Index].nibble_count + 1) / 2;
            cf->class.signature_pool[u2Index].type = (u1*)malloc(sizeof(u1) * array_type_count);
            if(cf->class.signature_pool[u2Index].type == NULL) {
                perror("parseClassComponent");
                return -1;
            }
            memcpy(cf->class.signature_pool[u2Index].type, data + position, sizeof(u1) * array_type_count);
            position += array_type_count;
            usedLength += (array_type_count + 1);
            offset += (array_type_count + 1);
            ++u2Index;
        }
    } else
        cf->class.can_have_signature_pool = 0;

    cf->class.interfaces_count = 0;
    cf->class.interfaces = NULL;
    cf->class.classes_count = 0;
    cf->class.classes = NULL;
    crtSize = position - initialPosition;       /* How much bytes were already used for this component */
    while(crtSize < size) {
        if((data[position] >> 4) & CLASS_ACC_INTERFACE) {     /* If it's an interface */
            cf_interface_info* tmp = NULL;
            u2 interfaces_count = cf->class.interfaces_count;
            u1 count = 0;
            cf->class.interfaces_count += 1;
            tmp = (cf_interface_info*)realloc(cf->class.interfaces, sizeof(cf_interface_info) * cf->class.interfaces_count);
            if(tmp == NULL) {
                perror("parseClassComponent");
                return -1;
            }
            cf->class.interfaces = tmp;

            cf->class.interfaces[interfaces_count].offset = crtSize;

            cf->class.interfaces[interfaces_count].flags = data[position] >> 4;
            count = data[position] & 0x0F;
            cf->class.interfaces[interfaces_count].interface_count = count;
            position += 1;
            cf->class.interfaces[interfaces_count].superinterfaces = (cf_class_ref_info*)malloc(sizeof(cf_class_ref_info) * count);
            if(cf->class.interfaces[interfaces_count].superinterfaces == NULL) {
                perror("parseClassComponent");
                return -1;
            }

            for(u1Index = 0; u1Index < count; ++u1Index) {
                cf->class.interfaces[interfaces_count].superinterfaces[u1Index].isExternal = (data[position] & 0x80) ? 1 : 0;
                if(cf->class.interfaces[interfaces_count].superinterfaces[u1Index].isExternal) {
                    cf->class.interfaces[interfaces_count].superinterfaces[u1Index].ref.external_class_ref.package_token = data[position++] & 0x7F;
                    cf->class.interfaces[interfaces_count].superinterfaces[u1Index].ref.external_class_ref.class_token = data[position++];
                } else {
                    cf->class.interfaces[interfaces_count].superinterfaces[u1Index].ref.internal_class_ref = bigEndianToU2(data + position);
                    position += 2;
                }
            }

            if(cf->class.interfaces[interfaces_count].flags & CLASS_ACC_REMOTE) {
                cf->class.interfaces[interfaces_count].has_interface_name = 1;
                cf->class.interfaces[interfaces_count].interface_name.interface_name_length = data[position++];
                cf->class.interfaces[interfaces_count].interface_name.interface_name = (u1*)malloc(sizeof(u1) * cf->class.interfaces[interfaces_count].interface_name.interface_name_length);
                if(cf->class.interfaces[interfaces_count].interface_name.interface_name == NULL) {
                    perror("parseClassComponent");
                    return -1;
                }

                memcpy(cf->class.interfaces[interfaces_count].interface_name.interface_name, data + position, sizeof(u1) * cf->class.interfaces[interfaces_count].interface_name.interface_name_length);
                position += cf->class.interfaces[interfaces_count].interface_name.interface_name_length;
            } else
                cf->class.interfaces[interfaces_count].has_interface_name = 0;
        } else {                                        /* If it's a class */
            cf_class_info* tmp = NULL;
            u2 classes_count = cf->class.classes_count;
            u1 count = 0;
            cf->class.classes_count += 1;
            tmp = (cf_class_info*)realloc(cf->class.classes, sizeof(cf_class_info) * cf->class.classes_count);
            if(tmp == NULL) {
                perror("parseClassComponent");
                return -1;
            }
            cf->class.classes = tmp;

            cf->class.classes[classes_count].offset = crtSize;

            cf->class.classes[classes_count].flags = data[position] >> 4;
            count = data[position] & 0x0F;
            cf->class.classes[classes_count].interface_count = count;
            position += 1;

            if((data[position] == 127) && (data[position + 1] == 127)) {
                cf->class.classes[classes_count].has_superclass = 0;
            } else {
                cf->class.classes[classes_count].has_superclass = 1;
                cf->class.classes[classes_count].super_class_ref.isExternal = (data[position] & 0x80) ? 1 : 0;
                if(cf->class.classes[classes_count].super_class_ref.isExternal) {
                    cf->class.classes[classes_count].super_class_ref.ref.external_class_ref.package_token = data[position++] & 0x7F;
                    cf->class.classes[classes_count].super_class_ref.ref.external_class_ref.class_token = data[position++];
                } else {
                    cf->class.classes[classes_count].super_class_ref.ref.internal_class_ref = bigEndianToU2(data + position);
                    position += 2;
                }
            }

            cf->class.classes[classes_count].declared_instance_size = data[position++];
            cf->class.classes[classes_count].first_reference_token = data[position++];
            cf->class.classes[classes_count].reference_count = data[position++];
            cf->class.classes[classes_count].public_method_table_base = data[position++];
            cf->class.classes[classes_count].public_method_table_count = data[position++];
            cf->class.classes[classes_count].package_method_table_base = data[position++];
            cf->class.classes[classes_count].package_method_table_count = data[position++];

            cf->class.classes[classes_count].public_virtual_method_table = (u2*)malloc(sizeof(u2) * cf->class.classes[classes_count].public_method_table_count);
            if(cf->class.classes[classes_count].public_virtual_method_table == NULL) {
                perror("parseClassComponent");
                return -1;
            }
            for(u1Index = 0; u1Index < cf->class.classes[classes_count].public_method_table_count; ++u1Index) {
                cf->class.classes[classes_count].public_virtual_method_table[u1Index] = bigEndianToU2(data + position);
                position += 2;
            }

            cf->class.classes[classes_count].package_virtual_method_table = (u2*)malloc(sizeof(u2) * cf->class.classes[classes_count].package_method_table_count);
            if(cf->class.classes[classes_count].package_virtual_method_table == NULL) {
                perror("parseClassComponent");
                return -1;
            }
            for(u1Index = 0; u1Index < cf->class.classes[classes_count].package_method_table_count; ++u1Index) {
                cf->class.classes[classes_count].package_virtual_method_table[u1Index] = bigEndianToU2(data + position);
                position += 2;
            }

            cf->class.classes[classes_count].interfaces = (cf_implemented_interface_info*)malloc(sizeof(cf_implemented_interface_info) * count);
            if(cf->class.classes[classes_count].interfaces == NULL) {
                perror("parseClassComponent");
                return -1;
            }

            for(u1Index = 0; u1Index < count; ++u1Index) {
                cf->class.classes[classes_count].interfaces[u1Index].interface.isExternal = (data[position] & 0x80) ? 1 : 0;
                if(cf->class.classes[classes_count].interfaces[u1Index].interface.isExternal) {
                    cf->class.classes[classes_count].interfaces[u1Index].interface.ref.external_class_ref.package_token = data[position++] & 0x7F;
                    cf->class.classes[classes_count].interfaces[u1Index].interface.ref.external_class_ref.class_token = data[position++];
                } else {
                    cf->class.classes[classes_count].interfaces[u1Index].interface.ref.internal_class_ref = bigEndianToU2(data + position);
                    position += 2;
                }
                cf->class.classes[classes_count].interfaces[u1Index].count = data[position++];
                cf->class.classes[classes_count].interfaces[u1Index].index = (u1*)malloc(sizeof(u1) * cf->class.classes[classes_count].interfaces[u1Index].count);
                if(cf->class.classes[classes_count].interfaces[u1Index].index == NULL) {
                    perror("parseClassComponent");
                    return -1;
                }
                memcpy(cf->class.classes[classes_count].interfaces[u1Index].index, data + position,sizeof(u1) * cf->class.classes[classes_count].interfaces[u1Index].count);
                position += cf->class.classes[classes_count].interfaces[u1Index].count;
            }

            if(cf->class.classes[classes_count].flags & CLASS_ACC_REMOTE) {
                cf->class.classes[classes_count].has_remote_interfaces = 1;
                cf->class.classes[classes_count].remote_interfaces.remote_methods_count = data[position++];
                cf->class.classes[classes_count].remote_interfaces.remote_methods = (cf_remote_method_info*)malloc(sizeof(cf_remote_method_info) * cf->class.classes[classes_count].remote_interfaces.remote_methods_count);
                if(cf->class.classes[classes_count].remote_interfaces.remote_methods == NULL) {
                    perror("parseClassComponent");
                    return -1;
                }
                for(u1Index = 0; u1Index < cf->class.classes[classes_count].remote_interfaces.remote_methods_count; ++u1Index) {
                    cf->class.classes[classes_count].remote_interfaces.remote_methods[u1Index].remote_method_hash = bigEndianToU2(data + position);
                    position += 2;
                    cf->class.classes[classes_count].remote_interfaces.remote_methods[u1Index].signature_offset = bigEndianToU2(data + position);
                    position += 2;
                    cf->class.classes[classes_count].remote_interfaces.remote_methods[u1Index].virtual_method_token = data[position++];
                }

                cf->class.classes[classes_count].remote_interfaces.hash_modifier_length = data[position++];
                cf->class.classes[classes_count].remote_interfaces.hash_modifier = (u1*)malloc(sizeof(u1) * cf->class.classes[classes_count].remote_interfaces.hash_modifier_length);
                if(cf->class.classes[classes_count].remote_interfaces.hash_modifier == NULL) {
                    perror("parseClassComponent");
                    return -1;
                }
                memcpy(cf->class.classes[classes_count].remote_interfaces.hash_modifier, data + position, sizeof(u1) * cf->class.classes[classes_count].remote_interfaces.hash_modifier_length);
                position += cf->class.classes[classes_count].remote_interfaces.hash_modifier_length;

                cf->class.classes[classes_count].remote_interfaces.class_name_length = data[position++];
                cf->class.classes[classes_count].remote_interfaces.class_name = (u1*)malloc(sizeof(u1) * cf->class.classes[classes_count].remote_interfaces.class_name_length);
                if(cf->class.classes[classes_count].remote_interfaces.class_name == NULL) {
                    perror("parseClassComponent");
                    return -1;
                }
                memcpy(cf->class.classes[classes_count].remote_interfaces.class_name, data + position, sizeof(u1) * cf->class.classes[classes_count].remote_interfaces.class_name_length);
                position += cf->class.classes[classes_count].remote_interfaces.class_name_length;

                cf->class.classes[classes_count].remote_interfaces.remote_interfaces_count = data[position++];
                cf->class.classes[classes_count].remote_interfaces.remote_interfaces = (cf_class_ref_info*)malloc(sizeof(cf_class_ref_info) * cf->class.classes[classes_count].remote_interfaces.remote_interfaces_count);
                if(cf->class.classes[classes_count].remote_interfaces.remote_interfaces == NULL) {
                    perror("parseClassComponent");
                    return -1;
                }
                for(u1Index = 0; u1Index < cf->class.classes[classes_count].remote_interfaces.remote_interfaces_count; ++u1Index) {
                    cf->class.classes[classes_count].remote_interfaces.remote_interfaces[u1Index].isExternal = (data[position] & 0x80) ? 1 : 0;
                    if(cf->class.classes[classes_count].remote_interfaces.remote_interfaces[u1Index].isExternal) {
                        cf->class.classes[classes_count].remote_interfaces.remote_interfaces[u1Index].ref.external_class_ref.package_token = data[position++] & 0x7F;
                        cf->class.classes[classes_count].remote_interfaces.remote_interfaces[u1Index].ref.external_class_ref.class_token = data[position++];
                    } else {
                        cf->class.classes[classes_count].remote_interfaces.remote_interfaces[u1Index].ref.internal_class_ref = bigEndianToU2(data + position);
                        position += 2;
                    }
                }
            } else
                cf->class.classes[classes_count].has_remote_interfaces = 0;
        }
        crtSize = position - initialPosition;
    }
 
    if(position != (cf->class.size + 3u)) {
        fprintf(stderr, "Parsing incomplete: %u != %u\n", position, cf->class.size + 3u);
        return -1;
    }

    return 0;

}


/**
 * \brief Parse the method component.
 *
 * The function parses the method component (see 6.9 Method Component of Virtual
 * Machine Specification, Java Card Platform, v2.2.2) from the input data and
 * output it straightforwardly into the method field of a cap_file struct. The
 * descriptor component should be read before calling this function.
 *
 * \param data The input data containing the raw method component.
 * \param cf   The structured representation of a cap file with a completed
 *             method component field.
 *
 * \return Return -1 if an error occured, 0 else.
 */
static int parseMethodComponent(cap_file* cf, char* data) {

    unsigned int position = 0;
    unsigned int initialPosition = 0;
    u2 crtSize = 0;
    u1 u1Index = 0;
    u2 u2Index = 0;
    u2 size = 0;
    u2 offset = 1;

    if(data[position] != COMPONENT_METHOD) {
        fprintf(stderr, "It should be a method component\n");
        return -1;
    }

    printf("Parsing method component\n");
    cf->method.tag = data[position++];
    cf->method.size = bigEndianToU2(data + position);
    position += 2;

    size = cf->method.size;
    initialPosition = position;

    cf->method.handler_count = data[position++];
    cf->method.exception_handlers = (cf_exception_handler_info*)malloc(sizeof(cf_exception_handler_info) * cf->method.handler_count);
    if(cf->method.exception_handlers == NULL) {
        perror("parseMethodComponent");
        return -1;
    }

    for(u1Index = 0; u1Index < cf->method.handler_count; ++u1Index) {
        u2 bitfield = 0;
        cf->method.exception_handlers[u1Index].start_offset = bigEndianToU2(data + position);
        position += 2;
        bitfield = bigEndianToU2(data + position);
        position += 2;
        cf->method.exception_handlers[u1Index].stop_bit = bitfield >> 15;
        cf->method.exception_handlers[u1Index].active_length = bitfield & 0x7F;
        cf->method.exception_handlers[u1Index].handler_offset = bigEndianToU2(data + position);
        position += 2;
        cf->method.exception_handlers[u1Index].catch_type_index = bigEndianToU2(data + position);
        position += 2;
    }

    offset += (cf->method.handler_count * 8);


    crtSize = position - initialPosition;
    cf->method.methods = NULL;
    cf->method.method_count = 0;
    while(crtSize < size) {
        u2 method_count = cf->method.method_count;
        u2 method_offset = position - 3;
        cf_method_info* tmp = NULL;
        ++cf->method.method_count;
        tmp = (cf_method_info*)realloc(cf->method.methods, sizeof(cf_method_info) * cf->method.method_count);
        if(tmp == NULL) {
            perror("parseMethodComponent");
            return -1;
        }
        cf->method.methods = tmp;

        cf->method.methods[method_count].offset = method_offset;

        cf->method.methods[method_count].method_header.flags = data[position] >> 4;
        if(cf->method.methods[method_count].method_header.flags & METHOD_ACC_EXTENDED) {
            cf->method.methods[method_count].method_header.extended_method_header.padding = data[position++] & 0x0F;
            cf->method.methods[method_count].method_header.extended_method_header.max_stack = data[position++];
            cf->method.methods[method_count].method_header.extended_method_header.nargs = data[position++];
            cf->method.methods[method_count].method_header.extended_method_header.max_locals = data[position++];
        } else {
            cf->method.methods[method_count].method_header.standard_method_header.max_stack = data[position++] & 0x0F;
            cf->method.methods[method_count].method_header.standard_method_header.nargs = data[position] >> 4;
            cf->method.methods[method_count].method_header.standard_method_header.max_locals = data[position++] & 0x0F;
        }

        cf->method.methods[method_count].bytecode_count = 0xDEAD;

        for(u1Index = 0; u1Index < cf->descriptor.class_count; ++u1Index) {
            for(u2Index = 0; u2Index < cf->descriptor.classes[u1Index].method_count; ++u2Index)
                if(cf->descriptor.classes[u1Index].methods[u2Index].method_offset == method_offset) {
                    cf->method.methods[method_count].bytecode_count = cf->descriptor.classes[u1Index].methods[u2Index].bytecode_count;
                    break;
                }

            if(u2Index != cf->descriptor.classes[u1Index].method_count)
               break; 
        }

        cf->method.methods[method_count].bytecodes = (u1*)malloc(sizeof(u1) * cf->method.methods[method_count].bytecode_count);
        if(cf->method.methods[method_count].bytecodes == NULL) {
            perror("parseMethodComponent");
            return -1;
        }
        memcpy(cf->method.methods[method_count].bytecodes, data + position, sizeof(u1) * cf->method.methods[method_count].bytecode_count);
        position += cf->method.methods[method_count].bytecode_count;

        crtSize = position - initialPosition;
        offset += ((cf->method.methods[method_count].method_header.flags == METHOD_ACC_EXTENDED ? 4 : 2) + cf->method.methods[u2Index].bytecode_count);
    }

    if(position != (cf->method.size + 3u)) {
        fprintf(stderr, "Parsing incomplete\n");
        return -1;
    }

    return 0;

}


/**
 * \brief Parse the static field component.
 *
 * The function parses the static field component (see 6.10 Static Field
 * Component of Virtual Machine Specification, Java Card Platform, v2.2.2) from
 * the input data and output it straightforwardly into the static_field field of
 * a cap_file struct.
 *
 * \param data The input data containing the raw static field component.
 * \param cf The structured representation of a cap file with a completed
 *           static field component field.
 *
 * \return Return -1 if an error occured, 0 else.
 */
static int parseStaticFieldComponent(cap_file* cf, char* data) {

    unsigned int position = 0;
    u2 u2Index = 0;

    if(data[position] != COMPONENT_STATICFIELD) {
        fprintf(stderr, "It should be a static field component\n");
        return -1;
    }

    printf("Parsing static field component\n");
    cf->static_field.tag = data[position++];
    cf->static_field.size = bigEndianToU2(data + position);
    position += 2;

    cf->static_field.image_size = bigEndianToU2(data + position);
    position += 2;

    cf->static_field.reference_count = bigEndianToU2(data + position);
    position += 2;

    cf->static_field.array_init_count = bigEndianToU2(data + position);
    position += 2;

    cf->static_field.array_init = (cf_array_init_info*)malloc(sizeof(cf_array_init_info) * cf->static_field.array_init_count);
    if(cf->static_field.array_init == NULL) {
        perror("parseStaticFieldComponent");
        return -1;
    }

    for(u2Index = 0; u2Index < cf->static_field.array_init_count; ++u2Index) {
        cf->static_field.array_init[u2Index].type = data[position++];
        cf->static_field.array_init[u2Index].count = bigEndianToU2(data + position);
        position += 2;
        cf->static_field.array_init[u2Index].values = (u1*)malloc(sizeof(u1) * cf->static_field.array_init[u2Index].count);
        if(cf->static_field.array_init[u2Index].values == NULL) {
            perror("parseStaticFieldComponent");
            return -1;
        }
        memcpy(cf->static_field.array_init[u2Index].values, data + position, sizeof(u1) * cf->static_field.array_init[u2Index].count);
        position += cf->static_field.array_init[u2Index].count;

    }

    cf->static_field.default_value_count = bigEndianToU2(data + position);
    position += 2;

    cf->static_field.non_default_value_count = bigEndianToU2(data + position);
    position += 2;

    cf->static_field.non_default_values = (u1*)malloc(sizeof(u1) * cf->static_field.non_default_value_count);
    if(cf->static_field.non_default_values == NULL) {
        perror("parseStaticFieldComponent");
        return -1;
    }
    memcpy(cf->static_field.non_default_values, data + position, sizeof(u1) * cf->static_field.non_default_value_count);
    position += cf->static_field.non_default_value_count;

    if(position != (cf->static_field.size + 3u)) {
        fprintf(stderr, "Parsing incomplete\n");
        return -1;
    }

   return 0;

}


/**
 * \brief Parse the reference location component.
 *
 * The function parses the reference location component (see 6.11 Reference
 * Location Component of Virtual Machine Specification, Java Card Platform,
 * v2.2.2) from the input data and output it straightforwardly into the
 * reference_location field of a cap_file struct.
 *
 * \param data The input data containing the raw reference location component.
 * \param cf The structured representation of a cap file with a completed
 *           reference location field component field.
 *
 * \return Return -1 if an error occured, 0 else.
 */
static int parseReferenceLocationComponent(cap_file* cf, char* data) {

    unsigned int position = 0;

    if(data[position] != COMPONENT_REFERENCELOCATION) {
        fprintf(stderr, "It should be a reference location component\n");
        return -1;
    }

    printf("Parsing reference location component\n");
    cf->reference_location.tag = data[position++];
    cf->reference_location.size = bigEndianToU2(data + position);
    position += 2;

    cf->reference_location.byte_index_count = bigEndianToU2(data + position);
    position += 2;
    cf->reference_location.offset_to_byte_indices = (u1*)malloc(sizeof(u1) * cf->reference_location.byte_index_count);
    if(cf->reference_location.offset_to_byte_indices == NULL) {
        perror("parseReferenceLocationComponent");
        return -1;
    }

    memcpy(cf->reference_location.offset_to_byte_indices, data + position, sizeof(u1) * cf->reference_location.byte_index_count);
    position += cf->reference_location.byte_index_count;

    cf->reference_location.byte2_index_count = bigEndianToU2(data + position);
    position += 2;
    cf->reference_location.offset_to_byte2_indices = (u1*)malloc(sizeof(u1) * cf->reference_location.byte2_index_count);
    if(cf->reference_location.offset_to_byte2_indices == NULL) {
        perror("parseReferenceLocationComponent");
        return -1;
    }
    memcpy(cf->reference_location.offset_to_byte2_indices, data + position, sizeof(u1) * cf->reference_location.byte2_index_count);
    position += cf->reference_location.byte2_index_count;

    if(position != (cf->reference_location.size + 3u)) {
        fprintf(stderr, "Parsing incomplete\n");
        return -1;
    }

    return 0;

}


/**
 * \brief Parse the export component.
 *
 * The function parses the export component (see 6.12 Export Component of
 * Virtual Machine Specification, Java Card Platform, v2.2.2) from the input
 * data and output it straightforwardly into the export field of a cap_file
 * struct.
 *
 * \param data The input data containing the raw export component.
 * \param cf The structured representation of a cap file with a completed
 *           export field component field.
 *
 * \return Return -1 if an error occured, 0 else.
 */
static int parseExportComponent(cap_file* cf, char* data) {

    unsigned int position = 0;
    u1 u1Index = 0;

    if(data[position] != COMPONENT_EXPORT) {
        fprintf(stderr, "It should be an export component\n");
        return -1;
    }

    printf("Parsing export component\n");
    cf->export.tag = data[position++];
    cf->export.size = bigEndianToU2(data + position);
    position += 2;
    cf->export.class_count = data[position++];

    cf->export.class_exports = (cf_class_export_info*)malloc(sizeof(cf_class_export_info) * cf->export.class_count);
    if(cf->export.class_exports == NULL) {
        perror("parseExportComponent");
        return -1;
    }

    for(u1Index = 0; u1Index < cf->export.class_count; ++u1Index) {
        u1 i = 0;
        cf->export.class_exports[u1Index].class_offset = bigEndianToU2(data + position);
        position += 2;
        cf->export.class_exports[u1Index].static_field_count = data[position++];
        cf->export.class_exports[u1Index].static_method_count = data[position++];

        cf->export.class_exports[u1Index].static_field_offsets = (u2*)malloc(sizeof(u2) * cf->export.class_exports[u1Index].static_field_count);
        if(cf->export.class_exports[u1Index].static_field_offsets == NULL) {
            perror("parseExportComponent");
            return -1;
        }
        for(; i < cf->export.class_exports[u1Index].static_field_count; ++i) {
            cf->export.class_exports[u1Index].static_field_offsets[i] = bigEndianToU2(data + position);
            position += 2;
        }

        cf->export.class_exports[u1Index].static_method_offsets = (u2*)malloc(sizeof(u2) * cf->export.class_exports[u1Index].static_method_count);
        if(cf->export.class_exports[u1Index].static_method_offsets == NULL) {
            perror("parseExportComponent");
            return -1;
        }
        for(i = 0; i < cf->export.class_exports[u1Index].static_method_count; ++i) {
            cf->export.class_exports[u1Index].static_method_offsets[i] = bigEndianToU2(data + position);
            position += 2;
        }

    }

    if(position != (cf->export.size + 3u)) {
        fprintf(stderr, "Parsing incomplete\n");
        return -1;
    }

    return 0;

}


/**
 * \brief Parse the descriptor component.
 *
 * The function parses the descriptor component (see 6.13 Descriptor Component
 * of Virtual Machine Specification, Java Card Platform, v2.2.2) from the input
 * data and output it straightforwardly into the descriptor field of a cap_file
 * struct.
 *
 * \param data The input data containing the raw descriptor component.
 * \param cf The structured representation of a cap file with a completed
 *           descriptor field component field.
 *
 * \return Return -1 if an error occured, 0 else.
 */
static int parseDescriptorComponent(cap_file* cf, char* data) {

    u1 u1Index = 0;
    u2 u2Index = 0;
    unsigned int position = 0;
    unsigned int initialPosition = 0;
    u2 crtSize = 0;
    u2 size = 0;
    u2 offset = 0;

    if(data[position] != COMPONENT_DESCRIPTOR) {
        fprintf(stderr, "It should be a descriptor component\n");
        return -1;
    }

    printf("Parsing descriptor component\n");
    cf->descriptor.tag = data[position++];
    cf->descriptor.size = bigEndianToU2(data + position);
    position += 2;

    size = cf->descriptor.size;
    initialPosition = position;

    cf->descriptor.class_count = data[position++];
    cf->descriptor.classes = (cf_class_descriptor_info*)malloc(sizeof(cf_class_descriptor_info) * cf->descriptor.class_count);
    if(cf->descriptor.classes == NULL) {
        perror("parseDescriptorComponent");
        return -1;
    }

    for(u1Index = 0; u1Index < cf->descriptor.class_count; ++u1Index) {
        u1 interfaceIndex = 0;
        cf->descriptor.classes[u1Index].token = data[position++];
        cf->descriptor.classes[u1Index].access_flags = data[position++];

        cf->descriptor.classes[u1Index].this_class_ref.isExternal = (data[position] & 0x80) ? 1 : 0;
        if(cf->descriptor.classes[u1Index].this_class_ref.isExternal) {
            cf->descriptor.classes[u1Index].this_class_ref.ref.external_class_ref.package_token = data[position++] & 0x7F;
            cf->descriptor.classes[u1Index].this_class_ref.ref.external_class_ref.class_token = data[position++];
        } else {
            cf->descriptor.classes[u1Index].this_class_ref.ref.internal_class_ref = bigEndianToU2(data + position);
            position += 2;
        }

        cf->descriptor.classes[u1Index].interface_count = data[position++];
        cf->descriptor.classes[u1Index].field_count = bigEndianToU2(data + position);
        position += 2;
        cf->descriptor.classes[u1Index].method_count = bigEndianToU2(data + position);
        position += 2;

        if(cf->descriptor.classes[u1Index].interface_count == 0) {
            cf->descriptor.classes[u1Index].interfaces = NULL;
        } else {
            cf->descriptor.classes[u1Index].interfaces = (cf_class_ref_info*)malloc(sizeof(cf_class_ref_info) * cf->descriptor.classes[u1Index].interface_count);
            if(cf->descriptor.classes[u1Index].interfaces == NULL) {
                perror("parseDescriptorComponent");
                return -1;
            }
            for(; interfaceIndex < cf->descriptor.classes[u1Index].interface_count; ++interfaceIndex) {
                cf->descriptor.classes[u1Index].interfaces[interfaceIndex].isExternal = (data[position] & 0x80) ? 1 : 0;
                if(cf->descriptor.classes[u1Index].interfaces[interfaceIndex].isExternal) {
                    cf->descriptor.classes[u1Index].interfaces[interfaceIndex].ref.external_class_ref.package_token = data[position++] & 0x7F;
                    cf->descriptor.classes[u1Index].interfaces[interfaceIndex].ref.external_class_ref.class_token = data[position++];
                } else {
                    cf->descriptor.classes[u1Index].interfaces[interfaceIndex].ref.internal_class_ref = bigEndianToU2(data + position);
                    position += 2;
                }
            }
        }

        if(cf->descriptor.classes[u1Index].field_count == 0) {
            cf->descriptor.classes[u1Index].fields = NULL;
        } else {
            cf->descriptor.classes[u1Index].fields = (cf_field_descriptor_info*)malloc(sizeof(cf_field_descriptor_info) * cf->descriptor.classes[u1Index].field_count);
            if(cf->descriptor.classes[u1Index].fields == NULL) {
                perror("parseDescriptorComponent");
                return -1;
            }
            for(u2Index = 0; u2Index < cf->descriptor.classes[u1Index].field_count; ++u2Index) {
                cf->descriptor.classes[u1Index].fields[u2Index].token = data[position++];
                cf->descriptor.classes[u1Index].fields[u2Index].access_flags = data[position++];

                if(cf->descriptor.classes[u1Index].fields[u2Index].access_flags & DESCRIPTOR_ACC_STATIC) {
                    cf->descriptor.classes[u1Index].fields[u2Index].field_ref.static_field.isExternal = data[position] & 0x80 ? 1 : 0;
                    if(cf->descriptor.classes[u1Index].fields[u2Index].field_ref.static_field.isExternal) {
                        cf->descriptor.classes[u1Index].fields[u2Index].field_ref.static_field.ref.external_ref.package_token = data[position++] & 0x7F;
                        cf->descriptor.classes[u1Index].fields[u2Index].field_ref.static_field.ref.external_ref.class_token = data[position++];
                        cf->descriptor.classes[u1Index].fields[u2Index].field_ref.static_field.ref.external_ref.token = data[position++];
                    } else {
                        cf->descriptor.classes[u1Index].fields[u2Index].field_ref.static_field.ref.internal_ref.padding = data[position++];
                        cf->descriptor.classes[u1Index].fields[u2Index].field_ref.static_field.ref.internal_ref.offset = bigEndianToU2(data + position);
                        position += 2;
                    }
                } else {
                    cf->descriptor.classes[u1Index].fields[u2Index].field_ref.instance_field.class_ref.isExternal = (data[position] & 0x80) ? 1 : 0;
                    if(cf->descriptor.classes[u1Index].fields[u2Index].field_ref.instance_field.class_ref.isExternal) {
                        cf->descriptor.classes[u1Index].fields[u2Index].field_ref.instance_field.class_ref.ref.external_class_ref.package_token = data[position++] & 0x7F;
                        cf->descriptor.classes[u1Index].fields[u2Index].field_ref.instance_field.class_ref.ref.external_class_ref.class_token = data[position++];
                    } else {
                        cf->descriptor.classes[u1Index].fields[u2Index].field_ref.instance_field.class_ref.ref.internal_class_ref = bigEndianToU2(data + position);
                        position += 2;
                    }
                    cf->descriptor.classes[u1Index].fields[u2Index].field_ref.instance_field.token = data[position++];
                }
                if(data[position] & 0x80)
                    cf->descriptor.classes[u1Index].fields[u2Index].type.primitive_type = bigEndianToU2(data + position);
                else
                    cf->descriptor.classes[u1Index].fields[u2Index].type.reference_type = bigEndianToU2(data + position);
                position += 2;
            }
        }

        if(cf->descriptor.classes[u1Index].method_count == 0) {
            cf->descriptor.classes[u1Index].methods = NULL;
        } else {
            cf->descriptor.classes[u1Index].methods = (cf_method_descriptor_info*)malloc(sizeof(cf_method_descriptor_info) * cf->descriptor.classes[u1Index].method_count);
            if(cf->descriptor.classes[u1Index].methods == NULL) {
                perror("parseDescriptorComponent");
                return -1;
            }
            for(u2Index = 0; u2Index < cf->descriptor.classes[u1Index].method_count; ++u2Index) {
                cf->descriptor.classes[u1Index].methods[u2Index].token = data[position++];
                cf->descriptor.classes[u1Index].methods[u2Index].access_flags = data[position++];

                cf->descriptor.classes[u1Index].methods[u2Index].method_offset = bigEndianToU2(data + position);
                position += 2;
                cf->descriptor.classes[u1Index].methods[u2Index].type_offset = bigEndianToU2(data + position);
                position += 2;
                cf->descriptor.classes[u1Index].methods[u2Index].bytecode_count = bigEndianToU2(data + position);
                position += 2;
                cf->descriptor.classes[u1Index].methods[u2Index].exception_handler_count = bigEndianToU2(data + position);
                position += 2;
                cf->descriptor.classes[u1Index].methods[u2Index].exception_handler_index = bigEndianToU2(data + position);
                position += 2;
            }
        }
    }

    cf->descriptor.types.constant_pool_count = bigEndianToU2(data + position);
    position += 2;

    cf->descriptor.types.constant_pool_types = (u2*)malloc(sizeof(u2) * cf->descriptor.types.constant_pool_count);
    if(cf->descriptor.types.constant_pool_types == NULL) {
        perror("parseDescriptorComponent");
        return -1;
    }
    for(u2Index = 0; u2Index < cf->descriptor.types.constant_pool_count; ++u2Index) {
        cf->descriptor.types.constant_pool_types[u2Index] = bigEndianToU2(data + position);
        position += 2;
    }

    crtSize = position - initialPosition;
    cf->descriptor.types.type_desc_count = 0;
    cf->descriptor.types.type_desc = NULL;

    offset = 2 + (cf->descriptor.types.constant_pool_count * 2);

    while(crtSize < size) {
        cf_type_descriptor* tmp = NULL;
        u2 type_desc_count = cf->descriptor.types.type_desc_count;
        u1 nibble_count = 0;
        ++cf->descriptor.types.type_desc_count;
        tmp = (cf_type_descriptor*)realloc(cf->descriptor.types.type_desc, sizeof(cf_type_descriptor) * cf->descriptor.types.type_desc_count);
        if(tmp == NULL) {
            perror("parseDescriptorComponent");
            return -1;
        }
        cf->descriptor.types.type_desc = tmp;
        cf->descriptor.types.type_desc[type_desc_count].offset = offset;
        nibble_count = data[position++];
        cf->descriptor.types.type_desc[type_desc_count].nibble_count = nibble_count;
        nibble_count = (nibble_count + 1) / 2;
        cf->descriptor.types.type_desc[type_desc_count].type = (u1*)malloc(sizeof(u1) * nibble_count);
        if(cf->descriptor.types.type_desc[type_desc_count].type == NULL) {
            perror("parseDescriptorComponent");
            return -1;
        }
        memcpy(cf->descriptor.types.type_desc[type_desc_count].type, data + position, sizeof(u1) * nibble_count);
        position += nibble_count;

        crtSize = position - initialPosition;
        offset += (1 + nibble_count);
    }

    if(position != (cf->descriptor.size + 3u)) {
        fprintf(stderr, "Descriptor: parsing incomplete\n");
        return -1;
    }

    return 0;

}


/**
 * \brief Parse the debug component.
 *
 * The function parses the debug component (see 6.14 debug Component of
 * Virtual Machine Specification, Java Card Platform, v2.2.2) from the input
 * data and output it straightforwardly into the debug field of a cap_file
 * struct.
 *
 * \param data The input data containing the raw debug component.
 * \param cf The structured representation of a cap file with a completed
 *           debug field component field.
 *
 * \return Return -1 if an error occured, 0 else.
 */
static int parseDebugComponent(cap_file* cf, char* data) {

    unsigned int position = 0;
    u2 u2Index = 0;

    if(data[position] != COMPONENT_DEBUG) {
        fprintf(stderr, "It should be a debug component\n");
        return -1;
    }

    printf("Parsing debug component\n");
    cf->debug.tag = data[position++];
    cf->debug.size = bigEndianToU2(data + position);
    position += 2;
    cf->debug.string_count = bigEndianToU2(data + position);
    position += 2;
    cf->debug.strings_table = (cf_utf8_info*)malloc(sizeof(cf_utf8_info) * cf->debug.string_count);
    if(cf->debug.strings_table == NULL) {
        perror("parseDebugComponent");
        return -1;
    }
    for(u2Index = 0; u2Index < cf->debug.string_count; ++u2Index) {
        cf->debug.strings_table[u2Index].length = bigEndianToU2(data + position);
        position += 2;
        cf->debug.strings_table[u2Index].bytes = (u1*)malloc(sizeof(u1) * cf->debug.strings_table[u2Index].length);
        if(cf->debug.strings_table[u2Index].bytes == NULL) {
            perror("parseDebugComponent");
            return -1;
        }
        memcpy(cf->debug.strings_table[u2Index].bytes, data + position, sizeof(u1) * cf->debug.strings_table[u2Index].length);
        position += cf->debug.strings_table[u2Index].length;
    }

    cf->debug.package_name_index = bigEndianToU2(data + position);
    position += 2;

    cf->debug.class_count = bigEndianToU2(data + position);
    position += 2;
    cf->debug.classes = (cf_class_debug_info*)malloc(sizeof(cf_class_debug_info) * cf->debug.class_count);
    if(cf->debug.classes == NULL) {
        perror("parseDebugComponent");
        return -1;
    }
    for(u2Index = 0; u2Index < cf->debug.class_count; ++u2Index) {
        u2 i = 0;
        cf->debug.classes[u2Index].name_index = bigEndianToU2(data + position);
        position += 2;

        cf->debug.classes[u2Index].access_flags = bigEndianToU2(data + position);
        position += 2;

        cf->debug.classes[u2Index].location = bigEndianToU2(data + position);
        position += 2;

        cf->debug.classes[u2Index].superclass_name_index = bigEndianToU2(data + position);
        position += 2;

        cf->debug.classes[u2Index].source_file_index = bigEndianToU2(data + position);
        position += 2;

        cf->debug.classes[u2Index].interface_count = data[position++];

        cf->debug.classes[u2Index].field_count = bigEndianToU2(data + position);
        position += 2;

        cf->debug.classes[u2Index].method_count = bigEndianToU2(data + position);
        position += 2;

        cf->debug.classes[u2Index].interface_names_indexes = (u2*)malloc(sizeof(u2) * cf->debug.classes[u2Index].interface_count);
        if(cf->debug.classes[u2Index].interface_names_indexes == NULL) {
            perror("parseDebugComponent");
            return -1;
        }
        for(; i < cf->debug.classes[u2Index].interface_count; ++i) {
            cf->debug.classes[u2Index].interface_names_indexes[i] = bigEndianToU2(data + position);
            position += 2;
        }

        cf->debug.classes[u2Index].fields = (cf_field_debug_info*)malloc(sizeof(cf_field_debug_info) * cf->debug.classes[u2Index].field_count);
        if(cf->debug.classes[u2Index].fields == NULL) {
            perror("parseDebugComponent");
            return -1;
        }
        for(i = 0; i < cf->debug.classes[u2Index].field_count; ++i) {
            cf->debug.classes[u2Index].fields[i].name_index = bigEndianToU2(data + position);
            position += 2;
            cf->debug.classes[u2Index].fields[i].descriptor_index = bigEndianToU2(data + position);
            position += 2;
            cf->debug.classes[u2Index].fields[i].access_flags = bigEndianToU2(data + position);
            position += 2;
            if((cf->debug.classes[u2Index].fields[i].access_flags & DEBUG_ACC_STATIC) &&
               (cf->debug.classes[u2Index].fields[i].access_flags & DEBUG_ACC_FINAL) &&
               ((cf->debug.strings_table[cf->debug.classes[u2Index].fields[i].descriptor_index].bytes[0] == 'B') ||
                (cf->debug.strings_table[cf->debug.classes[u2Index].fields[i].descriptor_index].bytes[0] == 'Z') ||
                (cf->debug.strings_table[cf->debug.classes[u2Index].fields[i].descriptor_index].bytes[0] == 'S') ||
                (cf->debug.strings_table[cf->debug.classes[u2Index].fields[i].descriptor_index].bytes[0] == 'I'))) {
                    cf->debug.classes[u2Index].fields[i].contents.const_value = bigEndianToU4(data + position);
                    position += 4;
            } else if((cf->debug.classes[u2Index].fields[i].access_flags & DEBUG_ACC_STATIC) &&
                          (!(cf->debug.classes[u2Index].fields[i].access_flags & DEBUG_ACC_FINAL) ||
                          (cf->debug.strings_table[cf->debug.classes[u2Index].fields[i].descriptor_index].bytes[0] == '['))) {
                cf->debug.classes[u2Index].fields[i].contents.location_var.pad = bigEndianToU2(data + position);
                position += 2;
                cf->debug.classes[u2Index].fields[i].contents.location_var.location = bigEndianToU2(data + position);
                position += 2;
            } else {
                cf->debug.classes[u2Index].fields[i].contents.token_var.pad1 = data[position++];
                cf->debug.classes[u2Index].fields[i].contents.token_var.pad2 = data[position++];
                cf->debug.classes[u2Index].fields[i].contents.token_var.pad3 = data[position++];
                cf->debug.classes[u2Index].fields[i].contents.token_var.token = data[position++];
            }
        }

        cf->debug.classes[u2Index].methods = (cf_method_debug_info*)malloc(sizeof(cf_method_debug_info) * cf->debug.classes[u2Index].method_count);
        if(cf->debug.classes[u2Index].methods == NULL) {
            perror("parseDebugComponent");
            return -1;
        }
        for(i = 0; i < cf->debug.classes[u2Index].method_count; ++i) {
            u2 j = 0;
            cf->debug.classes[u2Index].methods[i].name_index = bigEndianToU2(data + position);
            position += 2;
            cf->debug.classes[u2Index].methods[i].descriptor_index = bigEndianToU2(data + position);
            position += 2;
            cf->debug.classes[u2Index].methods[i].access_flags = bigEndianToU2(data + position);
            position += 2;
            cf->debug.classes[u2Index].methods[i].location = bigEndianToU2(data + position);
            position += 2;
            cf->debug.classes[u2Index].methods[i].header_size = data[position++];
            cf->debug.classes[u2Index].methods[i].body_size = bigEndianToU2(data + position);
            position += 2;
            cf->debug.classes[u2Index].methods[i].variable_count = bigEndianToU2(data + position);
            position += 2;
            cf->debug.classes[u2Index].methods[i].line_count = bigEndianToU2(data + position);
            position += 2;
            cf->debug.classes[u2Index].methods[i].variable_table = (cf_variable_info*)malloc(sizeof(cf_variable_info) * cf->debug.classes[u2Index].methods[i].variable_count);
            if(cf->debug.classes[u2Index].methods[i].variable_table == NULL) {
                perror("parseDebugComponent");
                return -1;
            }
            for(; j < cf->debug.classes[u2Index].methods[i].variable_count; ++j) {
                cf->debug.classes[u2Index].methods[i].variable_table[j].index = data[position++];
                cf->debug.classes[u2Index].methods[i].variable_table[j].name_index = bigEndianToU2(data + position);
                position += 2;
                cf->debug.classes[u2Index].methods[i].variable_table[j].descriptor_index = bigEndianToU2(data + position);
                position += 2;
                cf->debug.classes[u2Index].methods[i].variable_table[j].start_pc = bigEndianToU2(data + position);
                position += 2;
                cf->debug.classes[u2Index].methods[i].variable_table[j].length = bigEndianToU2(data + position);
                position += 2;
            }

            cf->debug.classes[u2Index].methods[i].line_table = (cf_line_info*)malloc(sizeof(cf_line_info) * cf->debug.classes[u2Index].methods[i].line_count);
            if(cf->debug.classes[u2Index].methods[i].line_table == NULL) {
                perror("parseDebugComponent");
                return -1;
            }
            for(j = 0; j < cf->debug.classes[u2Index].methods[i].line_count; ++j) {
                cf->debug.classes[u2Index].methods[i].line_table[j].start_pc = bigEndianToU2(data + position);
                position += 2;
                cf->debug.classes[u2Index].methods[i].line_table[j].end_pc = bigEndianToU2(data + position);
                position += 2;
                cf->debug.classes[u2Index].methods[i].line_table[j].source_line = bigEndianToU2(data + position);
                position += 2;
            }
        }
    }

    if(position != (cf->debug.size + 3u)) {
        fprintf(stderr, "Parsing incomplete\n");
        return -1;
    }

    return 0;

}


/**
 * \brief Read and parse a cap file.
 * 
 * Read a cap file located by filename, parse its component (excluding custom
 * ones) and return a straightforward representation.
 *
 * \param filename The cap file to read.
 *
 * \return An allocated cap_file structure containing the parsed cap file.
 */
cap_file* read_cap_file(const char* filename) {

    cap_file* cf = NULL;

    zip_uint64_t numEntries = 0;
    zip_uint64_t index = 0;
    int error = 0;
    struct zip* z = zip_open(filename, 0, &error);

    printf("Starting to read the cap file: %s\n", filename);

    if(error != 0) {
        char buf[1024];
        zip_error_to_str(buf, 1024, error, errno);
        fprintf(stderr, "%s\n", buf);
        return NULL;
    }

    cf = (cap_file*)calloc(1, sizeof(cap_file));
    if(cf == NULL) {
        perror("readCapFile");
        zip_close(z);
        return NULL;
    }

    numEntries = zip_get_num_entries(z, 0);

    for(; index < numEntries; ++index) {
        const char* name = zip_get_name(z, index, 0);
        char* substr = strrchr(name, '/');
        ++substr;
        if(strcmp(substr, "Header.cap") == 0) {
            char* data = readZipFile(z, name);
            if(data == NULL) {
                zip_close(z);
                return NULL;
            }

            if(parseHeaderComponent(cf, data) == -1) {
                free(data);
                zip_close(z);
                return NULL;
            } else {
                char* path = (char*)malloc(sizeof(char) * (substr - name + 1));
                if(path == NULL) {
                    perror("readCapFile");
                    free(data);
                    zip_close(z);
                    return NULL;
                }
                memcpy(path, name, sizeof(char) * (substr - name));
                path[substr - name] = '\0';
                cf->path = path;
                free(data);
            }
            break;
        }
    }

    if(index == numEntries) {
        fprintf(stderr, "Could not find the descriptor component\n");
        zip_close(z);
        return NULL;
    }

    if(cf->header.major_version != 2) {
        fprintf(stderr, "Wrong javacard major version. Was expecting 2, got %u.\n", cf->header.major_version);
        zip_close(z);
        return NULL;
    }

    for(index = 0; index < numEntries; ++index) {
        const char* name = zip_get_name(z, index, 0);
        char* substr = strrchr(name, '/');
        ++substr;
        if(strcmp(substr, "Descriptor.cap") == 0) {
            char* data = readZipFile(z, name);
            if(data == NULL) {
                zip_close(z);
                return NULL;
            }

            if(parseDescriptorComponent(cf, data) == -1) {
                free(data);
                zip_close(z);
                return NULL;
            } else
                free(data);
            break;
        }
    }

    if(index == numEntries) {
        fprintf(stderr, "Could not find the descriptor component\n");
        zip_close(z);
        return NULL;
    }

    for(index = 0; index < numEntries; ++index) {
        const char* name = zip_get_name(z, index, 0);
        char* substr = strrchr(name, '/');
        ++substr;
        if(strcmp(substr, "ConstantPool.cap") == 0) {
            char* data = readZipFile(z, name);
            if(data == NULL) {
                zip_close(z);
                return NULL;
            }

            if(parseConstantPoolComponent(cf, data) == -1) {
                free(data);
                zip_close(z);
                return NULL;
            } else
                free(data);
            break;
        }
    }

    if(index == numEntries) {
        fprintf(stderr, "Could not find the constant pool component\n");
        zip_close(z);
        return NULL;
    }


    for(index = 0; index < numEntries; ++index) {
        const char* name = zip_get_name(z, index, 0);
        char* data = NULL;
        char* substr = strrchr(name, '/');

        if(substr == NULL) {
            fprintf(stderr, "Wrong filename: %s\n", name);
            zip_close(z);
            return NULL;
        }

        ++substr;

        data = readZipFile(z, name);
        if(data == NULL) {
            zip_close(z);
            return NULL;
        }

        if(strcmp(name, "META-INF/MANIFEST.MF") == 0) {
            /* If the file we read from the zip was the manifest, we make it
               string */
            struct zip_stat stat;
            if(zip_stat_index(z, index, 0, &stat) == -1) {
                fprintf(stderr, "%s\n", zip_strerror(z));
                free(data);
                zip_close(z);
                return NULL;
            }
            if(!(stat.valid & ZIP_STAT_SIZE)) {
                fprintf(stderr, "Could not get manifest size\n");
                free(data);
                zip_close(z);
                return NULL;
            }
            cf->manifest = (char*)realloc(data, sizeof(char) * (stat.size + 1));
            if(cf->manifest == NULL) {
                perror("readCapFile");
                free(data);
                zip_close(z);
                return NULL;
            }
            cf->manifest[stat.size] = '\0';
            printf("Found manifest, skipping...\n");
        } else if(strcmp(substr, "Header.cap") == 0) {
            /* We already read the header component */
            printf("Skipping header...\n"); 
            free(data);
        } else if(strcmp(substr, "Directory.cap") == 0) {
            if(parseDirectoryComponent(cf, data) == -1) {
                free(data);
                zip_close(z);
                return NULL;
            }
            free(data);
        } else if(strcmp(substr, "Applet.cap") == 0) {
            if(parseAppletComponent(cf, data) == -1) {
                free(data);
                zip_close(z);
                return NULL;
            }
            free(data);
        } else if(strcmp(substr, "Import.cap") == 0) {
            if(parseImportComponent(cf, data) == -1) {
                free(data);
                zip_close(z);
                return NULL;
            }
            free(data);
        } else if(strcmp(substr, "ConstantPool.cap") == 0) {
            /* We already read the constant pool component */
            printf("Skipping constant pool...\n");
            free(data);
        } else if(strcmp(substr, "Class.cap") == 0) {
            if(parseClassComponent(cf, data) == -1) {
                free(data);
                zip_close(z);
                return NULL;
            }
            free(data);
        } else if(strcmp(substr, "Method.cap") == 0) {
            if(parseMethodComponent(cf, data) == -1) {
                free(data);
                zip_close(z);
                return NULL;
            }
            free(data);
        } else if(strcmp(substr, "StaticField.cap") == 0) {
            if(parseStaticFieldComponent(cf, data) == -1) {
                free(data);
                zip_close(z);
                return NULL;
            }
            free(data);
        } else if(strcmp(substr, "RefLocation.cap") == 0) {
            if(parseReferenceLocationComponent(cf, data) == -1) {
                free(data);
                zip_close(z);
                return NULL;
            }
            free(data);
        } else if(strcmp(substr, "Export.cap") == 0) {
            if(parseExportComponent(cf, data) == -1) {
                free(data);
                zip_close(z);
                return NULL;
            }
            free(data);
        } else if(strcmp(substr, "Descriptor.cap") == 0) {
            /* We already read the descriptor component */
            printf("Skipping descriptor...\n");
            free(data);
        } else if(strcmp(substr, "Debug.cap") == 0) {
            if(parseDebugComponent(cf, data) == -1) {
                free(data);
                zip_close(z);
                return NULL;
            }
            free(data);
        } else { 
            free(data);
            printf("Unsupported component, skipping...\n");
        }
    }

    zip_close(z);

    return cf;

}
