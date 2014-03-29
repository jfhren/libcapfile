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
 * \file analyzed_cap_file_verbose.c
 * \brief Output a human readable version of an analyzed CAP file.
 */

#include <stdio.h>

#include "analyzed_cap_file.h"
#include "bytecodes.h"

static void print_AID(u1* aid, u1 length) {

    u1 u1Index = 0;

    for(;u1Index < length; ++u1Index) {
        if(u1Index != 0)
            printf(":");
        printf("0x%.2X", aid[u1Index]);
    }

}


void verbose_constant_info(analyzed_cap_file* acf) {

    u1 u1Index = 0;

    printf("Constant info {\n");

    printf("\tjavacard_minor_version: %u\n", acf->info.javacard_minor_version);
    printf("\tjavacard_major_version: %u\n\n", acf->info.javacard_major_version);

    printf("\tpackage_minor_version: %u\n", acf->info.package_minor_version);
    printf("\tpackage_major_version: %u\n", acf->info.package_major_version);
    printf("\tpackage_aid_length: %u\n", acf->info.package_aid_length);
    printf("\tpackage_aid: ");
    print_AID(acf->info.package_aid, acf->info.package_aid_length);
    printf("\n");

    if(acf->info.has_package_name) {
        printf("\tpackage_name: %s\n", acf->info.package_name);
    }

    printf("\tcustom_count: %u\n", acf->info.custom_count);
    for(;u1Index < acf->info.custom_count; ++u1Index) {
        printf("\tcustom_components[%u] {\n", u1Index);
        printf("\t\ttag: %u\n", acf->info.custom_components[u1Index].tag);
        printf("\t\tsize: %u\n", acf->info.custom_components[u1Index].size);
        printf("\t\taid_length:%u\n", acf->info.custom_components[u1Index].aid_length);
        printf("\t\taid:");
        print_AID(acf->info.custom_components[u1Index].aid, acf->info.custom_components[u1Index].aid_length);
        printf("\n");
        printf("}\n");
    }
    printf("}\n");

}


void verbose_imported_package(analyzed_cap_file* acf) {

    u1 u1Index = 0;

    for(; u1Index < acf->imported_packages_count; ++u1Index) {
        printf("imported_packages[%u] {\n", u1Index);
        printf("\tmy_index: %u\n", acf->imported_packages[u1Index]->my_index);
        printf("\tcount: %u\n", acf->imported_packages[u1Index]->count);
        printf("\tminor_version: %u\n", acf->imported_packages[u1Index]->minor_version);
        printf("\tmajor_version: %u\n", acf->imported_packages[u1Index]->major_version);
        printf("\taid_length: %u\n", acf->imported_packages[u1Index]->aid_length);
        printf("\taid:");
        print_AID(acf->imported_packages[u1Index]->aid, acf->imported_packages[u1Index]->aid_length);
        printf("\n");
        printf("}\n");
    }

}


void print_one_type(one_type_descriptor_info* type) {

    if(type->type & TYPE_DESCRIPTOR_VOID)
        printf("void");
    else if(type->type & TYPE_DESCRIPTOR_BOOLEAN)
        printf("boolean");
    else if(type->type & TYPE_DESCRIPTOR_BYTE)
        printf("byte");
    else if(type->type & TYPE_DESCRIPTOR_SHORT)
        printf("short");
    else if(type->type & TYPE_DESCRIPTOR_INT)
        printf("int");
    else if(type->type & TYPE_DESCRIPTOR_REF)
        printf("ref");

    if(type->type & TYPE_DESCRIPTOR_ARRAY)
        printf("[]");

}


void print_type_descriptor(type_descriptor_info* desc, const char* prefix) {

    if(!desc) {
        printf("%ssignature: NULL\n", prefix);
        return;
    }

    if(desc->types_count > 1) {
        u1 u1Index = 0;
        printf("%ssignature: (", prefix);
        if(desc->types_count > 1) {
            for(; u1Index < (desc->types_count - 2); ++u1Index) {
                print_one_type(desc->types + u1Index);
                printf(", ");
            }
            print_one_type(desc->types + u1Index);
            ++u1Index;
        }
        printf(")");
        print_one_type(desc->types + u1Index);
        printf("\n");
    } else {
        printf("%stype: ", prefix);
        print_one_type(desc->types);
        printf("\n");
    }

}


void verbose_constant_pool(analyzed_cap_file* acf) {

    u2 u2Index = 0;

    for(;u2Index < acf->constant_pool_count; ++u2Index) {
        printf("constant_pool[%u] {\n", u2Index);

        if(acf->constant_pool[u2Index]->flags & CONSTANT_POOL_CLASSREF) {
            printf("\tCLASSREF {\n");
            printf("\t\tcount: %u\n", acf->constant_pool[u2Index]->count);
            printf("\t\tis_external: %u\n", (acf->constant_pool[u2Index]->flags & CONSTANT_POOL_IS_EXTERNAL) != 0);
            if(acf->constant_pool[u2Index]->flags & CONSTANT_POOL_IS_EXTERNAL) {
                printf("\t\texternal_package->my_index: %u\n", acf->constant_pool[u2Index]->external_package->my_index);
                printf("\t\texternal_class_token: %u\n", acf->constant_pool[u2Index]->external_class_token);
            } else {
                if(acf->constant_pool[u2Index]->internal_class)
                    printf("\t\tinternal_class->offset: %u\n", acf->constant_pool[u2Index]->internal_class->offset);
                else
                    printf("\t\tinternal_interface->offset: %u\n", acf->constant_pool[u2Index]->internal_interface->offset);
            }
        } else if (acf->constant_pool[u2Index]->flags & CONSTANT_POOL_INSTANCEFIELDREF) {
            printf("\tINSTANCEFIELDREF {\n");
            printf("\t\tis_external: %u\n", (acf->constant_pool[u2Index]->flags & CONSTANT_POOL_IS_EXTERNAL) != 0);
            printf("\t\tcount: %u\n", acf->constant_pool[u2Index]->count);
            print_type_descriptor(acf->constant_pool[u2Index]->type, "\t\t");
            if(acf->constant_pool[u2Index]->flags & CONSTANT_POOL_IS_EXTERNAL) {
                printf("\t\texternal_package->my_index: %u\n", acf->constant_pool[u2Index]->external_package->my_index);
                printf("\t\texternal_class_token: %u\n", acf->constant_pool[u2Index]->external_class_token);
                printf("\t\texternal_field_token: %u\n", acf->constant_pool[u2Index]->external_field_token);
            } else {
                printf("\t\tinternal_class->offset: %u\n", acf->constant_pool[u2Index]->internal_class->offset);
                printf("\t\tinternal_field->token: %u\n", acf->constant_pool[u2Index]->internal_field->token);
            }
        } else if (acf->constant_pool[u2Index]->flags & CONSTANT_POOL_VIRTUALMETHODREF) {
            printf("\tVIRTUALMETHODREF {\n");
            printf("\t\tis_external: %u\n", (acf->constant_pool[u2Index]->flags & CONSTANT_POOL_IS_EXTERNAL) != 0);
            print_type_descriptor(acf->constant_pool[u2Index]->type, "\t\t");
            if(acf->constant_pool[u2Index]->flags & CONSTANT_POOL_IS_EXTERNAL) {
                printf("\t\texternal_package->my_index: %u\n", acf->constant_pool[u2Index]->external_package->my_index);
                printf("\t\texternal_class_token: %u\n", acf->constant_pool[u2Index]->external_class_token);
                printf("\t\texternal_method_token: %u\n", acf->constant_pool[u2Index]->method_token);
            } else {
                printf("\t\tinternal_class->offset: %u\n", acf->constant_pool[u2Index]->internal_class->offset);
                printf("\t\tinternal_method->offset: %u\n", acf->constant_pool[u2Index]->internal_method->offset);
            }
        } else if (acf->constant_pool[u2Index]->flags & CONSTANT_POOL_SUPERMETHODREF) {
            printf("\tSUPERMETHODREF {\n");
            printf("\t\tis_external: %u\n", (acf->constant_pool[u2Index]->flags & CONSTANT_POOL_IS_EXTERNAL) != 0);
            print_type_descriptor(acf->constant_pool[u2Index]->type, "\t\t");
            if(acf->constant_pool[u2Index]->flags & CONSTANT_POOL_IS_EXTERNAL) {
                printf("\t\texternal_package->my_index: %u\n", acf->constant_pool[u2Index]->external_package->my_index);
                printf("\t\texternal_class_token: %u\n", acf->constant_pool[u2Index]->external_class_token);
            } else {
                printf("\t\tinternal_class->offset: %u\n", acf->constant_pool[u2Index]->internal_class->offset);
            }
            printf("\t\tmethod_token: %u\n", acf->constant_pool[u2Index]->method_token);
        } else if (acf->constant_pool[u2Index]->flags & CONSTANT_POOL_STATICFIELDREF) {
            printf("\tSTATICFIELDREF {\n");
            printf("\t\tis_external: %u\n", (acf->constant_pool[u2Index]->flags & CONSTANT_POOL_IS_EXTERNAL) != 0);
            printf("\t\tcount: %u\n", acf->constant_pool[u2Index]->count);
            print_type_descriptor(acf->constant_pool[u2Index]->type, "\t\t");
            if(acf->constant_pool[u2Index]->flags & CONSTANT_POOL_IS_EXTERNAL) {
                printf("\t\texternal_package->my_index: %u\n", acf->constant_pool[u2Index]->external_package->my_index);
                printf("\t\texternal_class_token: %u\n", acf->constant_pool[u2Index]->external_class_token);
                printf("\t\texternal_field_token: %u\n", acf->constant_pool[u2Index]->external_field_token);
            } else {
                printf("\t\tinternal_class->offset: %u\n", acf->constant_pool[u2Index]->internal_class->offset);
                printf("\t\tinternal_field->token: %u\n", acf->constant_pool[u2Index]->internal_field->token);
            }
        } else if (acf->constant_pool[u2Index]->flags & CONSTANT_POOL_STATICMETHODREF) {
            printf("\tSTATICMETHODREF {\n");
            printf("\t\tis_external: %u\n", (acf->constant_pool[u2Index]->flags & CONSTANT_POOL_IS_EXTERNAL) != 0);
            printf("\t\tcount: %u\n", acf->constant_pool[u2Index]->count);
            print_type_descriptor(acf->constant_pool[u2Index]->type, "\t\t");
            if(acf->constant_pool[u2Index]->flags & CONSTANT_POOL_IS_EXTERNAL) {
                printf("\t\texternal_package->my_index: %u\n", acf->constant_pool[u2Index]->external_package->my_index);
                printf("\t\texternal_class_token: %u\n", acf->constant_pool[u2Index]->external_class_token);
                printf("\t\texternal_method_token: %u\n", acf->constant_pool[u2Index]->method_token);
            } else {
                printf("\t\tinternal_class->offset: %u\n", acf->constant_pool[u2Index]->internal_class->offset);
                printf("\t\tinternal_method->offset: %u\n", acf->constant_pool[u2Index]->internal_method->offset);
            }
        }

        printf("\t}\n}\n");
    }

}


void verbose_signature_pool(analyzed_cap_file* acf) {

    u2 u2Index = 0;

    printf("signature poll {\n");

    for(; u2Index < acf->signature_pool_count; ++u2Index) {
        char prefix[16];

        sprintf(prefix, "\t%3u: ", u2Index);
        print_type_descriptor(acf->signature_pool[u2Index], prefix);
    }

    printf("}\n");

}


void print_bytecodes(bytecode_info** bytecodes, u2 bytecodes_count, const char* prefix) {

    u2 u2Index = 0;

    for(; u2Index < bytecodes_count; ++u2Index) {
        printf("%s\t%2u(%2u): %s", prefix, bytecodes[u2Index]->offset, bytecodes[u2Index]->info_offset, mnemonics[bytecodes[u2Index]->opcode]);
        if(bytecodes[u2Index]->opcode == 115) {
            u2 crt_case = 0;
            printf("{\n");
            for(; crt_case < bytecodes[u2Index]->stableswitch.nb_cases; ++crt_case)
                printf("%s\t\tcase %u: offset %u\n", prefix, bytecodes[u2Index]->stableswitch.low + crt_case, bytecodes[u2Index]->stableswitch.branches[crt_case]->offset);
            printf("%s\t\tdefault: offset %u\n%s\t};\n", prefix, bytecodes[u2Index]->stableswitch.default_branch->offset, prefix);
        } else if(bytecodes[u2Index]->opcode == 117) {
            u2 crt_case = 0;
            printf("{\n");
            for(; crt_case < bytecodes[u2Index]->slookupswitch.nb_cases; ++crt_case)
                printf("%s\t\tcase %d: offset %u\n", prefix, bytecodes[u2Index]->slookupswitch.cases[crt_case].match, bytecodes[u2Index]->slookupswitch.cases[crt_case].branch->offset);
            printf("%s\t\tdefault: offset %u\n%s\t};\n", prefix, bytecodes[u2Index]->slookupswitch.default_branch->offset, prefix);
        } else if(bytecodes[u2Index]->nb_args) {
            u1 u1Index = 0;
            for(;u1Index < bytecodes[u2Index]->nb_byte_args; ++u1Index)
                printf(" %.2X", bytecodes[u2Index]->args[u1Index]);
            if(bytecodes[u2Index]->has_ref)
                printf(" cp_ref: %u", bytecodes[u2Index]->ref->my_index);
            if(bytecodes[u2Index]->has_branch)
                printf(" branch: %u", bytecodes[u2Index]->branch->offset);
            printf(";\n");
        } else {
            printf(";\n");
        }
    }

}


void verbose_method(method_info* method, const char* prefix) {
    printf("%stoken: %u\n", prefix, method->token);
    printf("%soffset: %u\n", prefix, method->offset);
    printf("%sflags:", prefix);
    if(method->flags & METHOD_PUBLIC)
        printf(" PUBLIC");
    if(method->flags & METHOD_PRIVATE)
        printf(" PRIVATE");
    if(method->flags & METHOD_PROTECTED)
        printf(" PROTECTED");
    if(method->flags & METHOD_PACKAGE)
        printf(" PACKAGE");
    if(method->flags & METHOD_STATIC)
        printf(" STATIC");
    if(method->flags & METHOD_FINAL)
        printf(" FINAL");
    if(method->flags & METHOD_ABSTRACT)
        printf(" ABSTRACT");
    if(method->flags & METHOD_INIT)
        printf(" INIT");
    if(method->flags & METHOD_EXTENDED)
        printf(" EXTENDED");
    printf("\n");

    printf("%smax_stack: %u\n", prefix, method->max_stack);
    printf("%snargs: %u\n", prefix, method->nargs);
    printf("%smax_locals: %u\n", prefix, method->max_locals);

    if(method->is_overriding)
        printf("%sis_overriding\n", prefix);

    if(method->internal_overrided_method)
        printf("%sinternal_overrided_method: %u\n", prefix, method->internal_overrided_method->offset);

    print_type_descriptor(method->signature, prefix);
    printf("%sbytecode_count: %u\n", prefix, method->bytecodes_count);
    printf("%sbytecodes:\n", prefix);
    print_bytecodes(method->bytecodes, method->bytecodes_count, prefix);

}


void verbose_interfaces(analyzed_cap_file* acf) {

    u2 u2Index1 = 0;

    for(; u2Index1 < acf->interfaces_count; ++u2Index1) {
        u1 u1Index = 0;
        u2 u2Index2 = 0;
        printf("interfaces[%u] {\n", u2Index1);
        printf("\ttoken: %u\n", acf->interfaces[u2Index1]->token);
        printf("\toffset: %u\n", acf->interfaces[u2Index1]->offset);
        printf("\tflags:");
        if(acf->interfaces[u2Index1]->flags & INTERFACE_SHAREABLE)
            printf(" SHAREABLE");
        if(acf->interfaces[u2Index1]->flags & INTERFACE_REMOTE)
            printf(" REMOTE");
        if(acf->interfaces[u2Index1]->flags & INTERFACE_PUBLIC)
            printf(" PUBLIC");
        if(acf->interfaces[u2Index1]->flags & INTERFACE_PACKAGE)
            printf(" PACKAGE");
        if(acf->interfaces[u2Index1]->flags & INTERFACE_ABSTRACT)
            printf(" ABSTRACT");
        printf("\n");
        for(; u1Index < acf->interfaces[u2Index1]->superinterfaces_count; ++u1Index)
            printf("\tsuperinterfaces[%u]->my_index: %u\n", u1Index, acf->interfaces[u2Index1]->superinterfaces[u1Index]->my_index);
        for(; u2Index2 < acf->interfaces[u2Index1]->methods_count; ++u2Index2) {
            printf("\tmethods[%u] {\n", u2Index2);
            verbose_method(acf->interfaces[u2Index1]->methods[u2Index2], "\t\t");
            printf("\t}\n");
        }
        printf("}\n");
    }

}


void printAID(u1* aid, u1 length) {

    u1 u1Index = 0;

    for(;u1Index < length; ++u1Index) {
        if(u1Index != 0)
            printf(":");
        printf("0x%.2X", aid[u1Index]);
    }

}


void verbose_field(field_info* field, const char* prefix) {

    printf("%stoken: %u\n", prefix, field->token);
    printf("%sflags:", prefix);
    if(field->flags & FIELD_PUBLIC)
        printf(" PUBLIC");
    if(field->flags & FIELD_PRIVATE)
        printf(" PRIVATE");
    if(field->flags & FIELD_PROTECTED)
        printf(" PROTECTED");
    if(field->flags & FIELD_PACKAGE)
        printf(" PACKAGE");
    if(field->flags & FIELD_STATIC)
        printf(" STATIC");
    if(field->flags & FIELD_FINAL)
        printf(" FINAL");
    printf("\n");

    print_type_descriptor(field->type, prefix);

    if(field->flags & FIELD_HAS_VALUE) {
        u2 u2Index = 0;
        printf("%svalue:", prefix);
        for(; u2Index < field->value_size; ++u2Index)
            printf(" %.2X", field->value[u2Index]);
        printf("\n");
    }

}


void verbose_classes(analyzed_cap_file* acf) {

    u2 u2Index1 = 0;

    for(; u2Index1 < acf->classes_count; ++u2Index1) {
        u1 u1Index1 = 0;
        u2 u2Index2 = 0;

        printf("classes[%u] {\n", u2Index1);

        printf("\ttoken: %u\n", acf->classes[u2Index1]->token);
        printf("\toffset: %u\n", acf->classes[u2Index1]->offset);

        printf("\tflags:");
        if(acf->classes[u2Index1]->flags & CLASS_PUBLIC)
            printf(" PUBLIC");
        if(acf->classes[u2Index1]->flags & CLASS_PACKAGE)
            printf(" PACKAGE");
        if(acf->classes[u2Index1]->flags & CLASS_FINAL)
            printf(" FINAL");
        if(acf->classes[u2Index1]->flags & CLASS_ABSTRACT)
            printf(" ABSTRACT");
        if(acf->classes[u2Index1]->flags & CLASS_SHAREABLE)
            printf(" SHAREABLE");
        if(acf->classes[u2Index1]->flags & CLASS_REMOTE)
            printf(" REMOTE");
        printf("\n");

        if(acf->classes[u2Index1]->flags & CLASS_APPLET) {
            printf("\tAID: ");
            printAID(acf->classes[u2Index1]->aid, acf->classes[u2Index1]->aid_length);
            printf("\n");
            printf("\tinstall_method->offset: %u\n", acf->classes[u2Index1]->install_method->offset);
        }

        if(acf->classes[u2Index1]->superclass)
            printf("\tsuperclass->my_index: %u\n", acf->classes[u2Index1]->superclass->my_index);

        for(; u1Index1 < acf->classes[u2Index1]->interfaces_count; ++u1Index1) {
            u1 u1Index2 = 0;

            printf("\tinterfaces[%u] {\n", u1Index1);
            printf("\t\tref->my_index: %u\n", acf->classes[u2Index1]->interfaces[u1Index1].ref->my_index);

            for(; u1Index2 < acf->classes[u2Index1]->interfaces[u1Index1].count; ++u1Index2) {
                printf("\t\tindex[%u] {\n", u1Index2);
                if(acf->classes[u2Index1]->interfaces[u1Index1].index[u1Index2].declaration)
                    printf("\t\t\tdeclaration->offset: %u\n", acf->classes[u2Index1]->interfaces[u1Index1].index[u1Index2].declaration->offset);
                if(acf->classes[u2Index1]->interfaces[u1Index1].index[u2Index2].implementation)
                    printf("\t\t\timplementation->offset: %u\n", acf->classes[u2Index1]->interfaces[u1Index1].index[u1Index2].implementation->offset);
            }
            printf("}\n");
        }

        for(; u2Index2 < acf->classes[u2Index1]->fields_count; ++u2Index2) {
            printf("\tfields[%u] {\n", u2Index2);
            verbose_field(acf->classes[u2Index1]->fields[u2Index2], "\t\t");
            printf("\t}\n");
        }

        for(u2Index2 = 0; u2Index2 < acf->classes[u2Index1]->methods_count; ++u2Index2) {
            printf("\tmethods[%u] {\n", u2Index2);
            verbose_method(acf->classes[u2Index1]->methods[u2Index2], "\t\t");
            printf("\t}\n");
        }

        printf("}\n");
    }

}


void verbose_exception_handlers(analyzed_cap_file* acf) {

    u1 u1Index = 0;

    for(; u1Index < acf->exception_handlers_count; ++u1Index) {
        printf("exception_handlers[%u] {\n", u1Index);
        printf("\tstop_bit: %u\n", acf->exception_handlers[u1Index]->stop_bit);
        if(acf->exception_handlers[u1Index]->start)
            printf("\tstart->offset: %u\n", acf->exception_handlers[u1Index]->start->info_offset);
        if(acf->exception_handlers[u1Index]->end)
            printf("\tend->offset: %u\n", acf->exception_handlers[u1Index]->end->info_offset);
        if(acf->exception_handlers[u1Index]->handler)
            printf("\thandler->offset: %u\n", acf->exception_handlers[u1Index]->handler->info_offset);
        if(acf->exception_handlers[u1Index]->catch_type)
            printf("\tcatch_type->my_index: %u\n", acf->exception_handlers[u1Index]->catch_type->my_index);
        printf("}\n");
    }

}
