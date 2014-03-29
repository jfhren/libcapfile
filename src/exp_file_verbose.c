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
 * \file exp_file_verbose.c
 * \brief Output a human readable version of an export file.
 */

#include <stdio.h>
#include "exp_file.h"

static void print_AID(u1* aid, u1 length) {

    u1 u1Index = 0;

    for(;u1Index < length; ++u1Index) {
        if(u1Index != 0)
            printf(":");
        printf("0x%.2X", aid[u1Index]);
    }

}


void verbose_export_file(export_file* ef) {

    u1 u1Index1 = 0;
    u2 u2Index1 = 0;

    printf("export_file {\n");
    printf("\tmagic: 0x%X\n", ef->magic);
    printf("\tminor_version: %u\n", ef->minor_version);
    printf("\tmajor_version: %u\n", ef->major_version);

    for(; u2Index1 < ef->constant_pool_count; ++u2Index1) {
        printf("\tconstant_pool[%u] {\n", u2Index1);
        switch(ef->constant_pool[u2Index1].tag) {
            case EF_CONSTANT_PACKAGE:
                printf("\t\tCONSTANT_Package.flags: ");
                if(ef->constant_pool[u2Index1].CONSTANT_Package.flags & EF_ACC_LIBRARY)
                    printf("ACC_LIBRARY");
                printf("\n");

                printf("\t\tCONSTANT_Package.name_index:  %u\n", ef->constant_pool[u2Index1].CONSTANT_Package.name_index);
                printf("\t\tCONSTANT_Package.minor_version: %u\n", ef->constant_pool[u2Index1].CONSTANT_Package.minor_version);
                printf("\t\tCONSTANT_Package.major_version: %u\n", ef->constant_pool[u2Index1].CONSTANT_Package.major_version);
                printf("\t\tCONSTANT_Package.aid_length: %u\n", ef->constant_pool[u2Index1].CONSTANT_Package.aid_length);
                printf("\t\tCONSTANT_Package.aid: ");
                print_AID(ef->constant_pool[u2Index1].CONSTANT_Package.aid, ef->constant_pool[u2Index1].CONSTANT_Package.aid_length);
                printf("\n");
                break;

            case EF_CONSTANT_CLASSREF:
                printf("\t\tCONSTANT_Classref.name_index: %u\n", ef->constant_pool[u2Index1].CONSTANT_Classref.name_index);
                break;

            case EF_CONSTANT_INTEGER:
                printf("\t\tCONSTANT_Integer.bytes: %u\n", ef->constant_pool[u2Index1].CONSTANT_Integer.bytes);
                break;

            case EF_CONSTANT_UTF8:
                printf("\t\tCONSTANT_Utf8.length: %u\n", ef->constant_pool[u2Index1].CONSTANT_Utf8.length);
                printf("\t\tCONSTANT_Utf8.bytes: %.*s\n", ef->constant_pool[u2Index1].CONSTANT_Utf8.length, ef->constant_pool[u2Index1].CONSTANT_Utf8.bytes);
                break;

        }
        printf("\t}\n");
    }

    printf("\tthis_package: %u\n", ef->this_package);

    for(; u1Index1 < ef->export_class_count; ++u1Index1) {
        u1 u1Index2 = 0;

        printf("\tclasses[%u] {\n", u1Index1);
        printf("\t\ttoken: %u\n", ef->classes[u1Index1].token);

        printf("\t\taccess_flags:");
        if(ef->classes[u1Index1].access_flags & EF_ACC_PUBLIC)
            printf(" ACC_PUBLIC");
        if(ef->classes[u1Index1].access_flags & EF_ACC_FINAL)
            printf(" ACC_FINAL");
        if(ef->classes[u1Index1].access_flags & EF_ACC_INTERFACE)
            printf(" ACC_INTERFACE");
        if(ef->classes[u1Index1].access_flags & EF_ACC_ABSTRACT)
            printf(" ACC_ABSTRACT");
        if(ef->classes[u1Index1].access_flags & EF_ACC_SHAREABLE)
            printf(" ACC_SHAREABLE");
        if(ef->classes[u1Index1].access_flags & EF_ACC_REMOTE)
            printf(" ACC_REMOTE");
        printf("\n");

        printf("\t\tname_index: %u // %.*s\n", ef->classes[u1Index1].name_index, ef->constant_pool[ef->constant_pool[ef->classes[u1Index1].name_index].CONSTANT_Classref.name_index].CONSTANT_Utf8.length, ef->constant_pool[ef->constant_pool[ef->classes[u1Index1].name_index].CONSTANT_Classref.name_index].CONSTANT_Utf8.bytes);

        printf("\t\tsupers:");
        for(u2Index1 = 0; u2Index1 < ef->classes[u1Index1].export_supers_count; ++u2Index1)
            printf(" %u", ef->classes[u1Index1].supers[u2Index1]);
        printf("\n");

        printf("\t\tinterfaces:");
        for(; u1Index2 < ef->classes[u1Index1].export_interfaces_count; ++u1Index2)
            printf(" %u", ef->classes[u1Index1].interfaces[u1Index2]);
        printf("\n");

        for(u2Index1 = 0; u2Index1 < ef->classes[u1Index1].export_fields_count; ++u2Index1) {
            u2 u2Index2 = 0;

            printf("\t\tfields[%u] {\n", u2Index1);

            printf("\t\t\ttoken: %u\n", ef->classes[u1Index1].fields[u2Index1].token);

            printf("\t\t\taccess_flags:");
            if(ef->classes[u1Index1].fields[u2Index1].access_flags & EF_ACC_PUBLIC)
                printf(" ACC_PUBLIC");
            if(ef->classes[u1Index1].fields[u2Index1].access_flags & EF_ACC_PROTECTED)
                printf(" ACC_PROTECTED");
            if(ef->classes[u1Index1].fields[u2Index1].access_flags & EF_ACC_STATIC)
                printf(" ACC_STATIC");
            if(ef->classes[u1Index1].fields[u2Index1].access_flags & EF_ACC_FINAL)
                printf(" ACC_FINAL");
            printf("\n");

            printf("\t\t\tname_index: %u // %.*s\n", ef->classes[u1Index1].fields[u2Index1].name_index, ef->constant_pool[ef->classes[u1Index1].fields[u2Index1].name_index].CONSTANT_Utf8.length, ef->constant_pool[ef->classes[u1Index1].fields[u2Index1].name_index].CONSTANT_Utf8.bytes);

            printf("\t\t\tdescriptor_index: %u\n", ef->classes[u1Index1].fields[u2Index1].descriptor_index);

            for(; u2Index2 < ef->classes[u1Index1].fields[u2Index1].attributes_count; ++u2Index2) {
                printf("\t\t\tattributes[%u] {\n", u2Index2);
                printf("\t\t\t\tattribute_name_index: %u\n", ef->classes[u1Index1].fields[u2Index1].attributes[u2Index2].attribute_name_index);
                printf("\t\t\t\tattribute_length: %u\n", ef->classes[u1Index1].fields[u2Index1].attributes[u2Index2].attribute_length);
                printf("\t\t\t\tconstantvalue_index: %u\n", ef->classes[u1Index1].fields[u2Index1].attributes[u2Index2].constantvalue_index);
                printf("\t\t\t}\n");
            }

            printf("\t\t}\n");
        }

        for(u2Index1 = 0; u2Index1 < ef->classes[u1Index1].export_methods_count; ++u2Index1) {
            printf("\t\tmethods[%u] {\n", u2Index1);
            printf("\t\t\ttoken: %u\n", ef->classes[u1Index1].methods[u2Index1].token);
            printf("\t\t\taccess_flags:");
            if(ef->classes[u1Index1].methods[u2Index1].access_flags & EF_ACC_PUBLIC)
                printf(" ACC_PUBLIC");
            if(ef->classes[u1Index1].methods[u2Index1].access_flags & EF_ACC_PROTECTED)
                printf(" ACC_PROTECTED");
            if(ef->classes[u1Index1].methods[u2Index1].access_flags & EF_ACC_STATIC)
                printf(" ACC_STATIC");
            if(ef->classes[u1Index1].methods[u2Index1].access_flags & EF_ACC_FINAL)
                printf(" ACC_FINAL");
            if(ef->classes[u1Index1].methods[u2Index1].access_flags & EF_ACC_ABSTRACT)
                printf(" ACC_ABSTRACT");
            printf("\n");
            printf("\t\t\tname_index: %u // %.*s\n", ef->classes[u1Index1].methods[u2Index1].name_index, ef->constant_pool[ef->classes[u1Index1].methods[u2Index1].name_index].CONSTANT_Utf8.length, ef->constant_pool[ef->classes[u1Index1].methods[u2Index1].name_index].CONSTANT_Utf8.bytes);
            printf("\t\t\tdescriptor_index: %u\n", ef->classes[u1Index1].methods[u2Index1].descriptor_index);
            printf("\t\t}\n");
        }

        printf("\t}\n");
    }

    printf("}\n");

}
