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
 * \file cap_file_verbose.c
 * \brief Output a human readble version of a CAP file in its straightforward
 * representation.  
 */

#include <stdio.h>
#include "cap_file.h"


static void print_AID(u1* aid, u1 length) {

    u1 u1Index = 0;

    for(;u1Index < length; ++u1Index) {
        if(u1Index != 0)
            printf(":");
        printf("0x%.2X", aid[u1Index]);
    }

}


void verbose_manifest(cap_file* cf) {

    printf("Manifest {\n");
    printf("%s", cf->manifest);
    printf("}\n");

}


void verbose_header_component(cap_file* cf) {

    printf("header_component {\n");
    if(cf->header.tag != 0) {
        printf("\ttag: %u\n", cf->header.tag);
        printf("\tsize: %u\n", cf->header.size);
        printf("\tmagic: %X\n", cf->header.magic);
        printf("\tminor_version: %u\n", cf->header.minor_version);
        printf("\tmajor_version: %u\n", cf->header.major_version);
        printf("\tflags:");
        if(cf->header.flags & HEADER_ACC_INT)
            printf(" ACC_INT");
        if(cf->header.flags & HEADER_ACC_EXPORT)
            printf(" ACC_EXPORT");
        if(cf->header.flags & HEADER_ACC_APPLET)
            printf(" ACC_APPLET");
        printf("\n");
        printf("\tpackage {\n");
            printf("\t\tminor_version: %u\n", cf->header.package.minor_version);
            printf("\t\tmajor_version: %u\n", cf->header.package.major_version);
            printf("\t\tAID_length: %u\n", cf->header.package.AID_length);
            printf("\t\tAID: ");
            print_AID(cf->header.package.AID, cf->header.package.AID_length);
            printf("\n");
        printf("\t}\n");
        if(cf->header.has_package_name) {
        printf("\tpackage_name {\n");
            printf("\t\tname_length: %u\n", cf->header.package_name.name_length);
            printf("\t\tname: %.*s\n", (int)cf->header.package_name.name_length, cf->header.package_name.name);
        printf("\t}\n");
        }
    }
    printf("}\n");

}

void verbose_directory_component(cap_file* cf) {

    printf("directory_component {\n");
    if(cf->directory.tag != 0) {
        printf("\ttag: %u\n", cf->directory.tag);
        printf("\tsize: %u\n", cf->directory.size);
        printf("\tcomponent_sizes[COMPONENT_Header]: %u\n", cf->directory.component_sizes[0]);
        printf("\tcomponent_sizes[COMPONENT_Directory]: %u\n", cf->directory.component_sizes[1]);
        printf("\tcomponent_sizes[COMPONENT_Applet]: %u\n", cf->directory.component_sizes[2]);
        printf("\tcomponent_sizes[COMPONENT_Import]: %u\n", cf->directory.component_sizes[3]);
        printf("\tcomponent_sizes[COMPONENT_ConstantPool]: %u\n", cf->directory.component_sizes[4]);
        printf("\tcomponent_sizes[COMPONENT_Class]: %u\n", cf->directory.component_sizes[5]);
        printf("\tcomponent_sizes[COMPONENT_Method]: %u\n", cf->directory.component_sizes[6]);
        printf("\tcomponent_sizes[COMPONENT_StaticField]: %u\n", cf->directory.component_sizes[7]);
        printf("\tcomponent_sizes[COMPONENT_ReferenceLocation]: %u\n", cf->directory.component_sizes[8]);
        printf("\tcomponent_sizes[COMPONENT_Export]: %u\n", cf->directory.component_sizes[9]);
        printf("\tcomponent_sizes[COMPONENT_Descriptor]: %u\n", cf->directory.component_sizes[10]);
        if(cf->directory.can_have_debug_component)
        printf("\tcomponent_sizes[COMPONENT_Debug]: %u\n", cf->directory.component_sizes[11]);
        printf("\tstatic_field_size {\n");
            printf("\t\timage_size: %u\n", cf->directory.static_field_size.image_size);
            printf("\t\tarray_init_count: %u\n", cf->directory.static_field_size.array_init_count);
            printf("\t\tarray_init_size: %u\n", cf->directory.static_field_size.array_init_size);
        printf("\t}\n");
        printf("\timport_count: %u\n", cf->directory.import_count);
        printf("\tapplet_count: %u\n", cf->directory.applet_count);
        printf("\tcustom_count: %u\n", cf->directory.custom_count);
        if(cf->directory.custom_count != 0) {
            u1 u1Index = 0;
            for(;u1Index < cf->directory.custom_count; ++u1Index) {
        printf("\tcustom_components[%u] {\n", u1Index);
            printf("\t\tcomponent_tag: %u\n", cf->directory.custom_components[u1Index].component_tag);
            printf("\t\tsize: %u\n", cf->directory.custom_components[u1Index].size);
            printf("\t\tAID_length: %u\n", cf->directory.custom_components[u1Index].AID_length);
            printf("\t\tAID: ");
            print_AID(cf->directory.custom_components[u1Index].AID, cf->directory.custom_components[u1Index].AID_length);
            printf("\n");
        printf("\t}\n");
            }
        }
    }
    printf("}\n");

}


void verbose_applet_component(cap_file* cf) {

    printf("applet_component {\n");
    if(cf->applet.tag != 0) {
        u1 u1Index = 0;
        printf("\ttag: %u\n", cf->applet.tag);
        printf("\tsize: %u\n", cf->applet.size);
        printf("\tcount: %u\n", cf->applet.count);
        for(; u1Index < cf->applet.count; ++u1Index) {
        printf("\tapplets[%u] {\n", u1Index);
            printf("\t\tAID_length: %u\n",cf->applet.applets[u1Index].AID_length);
            printf("\t\tAID: ");
            print_AID(cf->applet.applets[u1Index].AID, cf->applet.applets[u1Index].AID_length);
            printf("\n");
            printf("\t\tinstall_method_offset: %u\n", cf->applet.applets[u1Index].install_method_offset);
        printf("\t}\n");
        }
    }
    printf("}\n");

}


void verbose_import_component(cap_file* cf) {

    printf("import_component {\n");
    if(cf->import.tag != 0) {
        u1 u1Index = 0;
        printf("\ttag: %u\n", cf->import.tag);
        printf("\tsize: %u\n", cf->import.size);
        printf("\tcount: %u\n", cf->import.count);
        for(; u1Index < cf->import.count; ++u1Index) {
        printf("\tpackages[%u] {\n", u1Index);
            printf("\t\tminor_version: %u\n", cf->import.packages[u1Index].minor_version);
            printf("\t\tmajor_version: %u\n", cf->import.packages[u1Index].major_version);
            printf("\t\tAID_length: %u\n", cf->import.packages[u1Index].AID_length);
            printf("\t\tAID: ");
            print_AID(cf->import.packages[u1Index].AID, cf->import.packages[u1Index].AID_length);
            printf("\n");
        printf("\t}\n");
        }
    }
    printf("}\n");

}


static void print_classref(cf_class_ref_info* class_ref, const char* prefix) {

    printf("%sclass_ref {\n", prefix);
    if(class_ref->isExternal) {
        printf("%s\texternal_class_ref {\n", prefix);
            printf("%s\t\tpackage_token: %u\n", prefix, class_ref->ref.external_class_ref.package_token);
            printf("%s\t\tclass_token: %u\n", prefix, class_ref->ref.external_class_ref.class_token);
        printf("%s\t}\n", prefix);
    } else {
        printf("%s\tinternal_class_ref: %u\n", prefix, class_ref->ref.internal_class_ref);
    }
    printf("%s}\n", prefix);

}


static void print_staticref(cf_static_ref_info* static_ref, const char* which, const char* prefix) {

    printf("%sstatic_%s_ref {\n", prefix, which);
    if(static_ref->isExternal) {
        printf("%s\texternal_ref {\n", prefix);
            printf("%s\t\tpackage_token: %u\n", prefix, static_ref->ref.external_ref.package_token);
            printf("%s\t\tclass_token: %u\n", prefix, static_ref->ref.external_ref.class_token);
            printf("%s\t\ttoken: %u\n", prefix, static_ref->ref.external_ref.token);
        printf("%s\t}\n", prefix);
    } else {
        printf("%s\tinternal_ref {\n", prefix);
            printf("%s\t\tpadding: %u\n", prefix, static_ref->ref.internal_ref.padding);
            printf("%s\t\toffset: %u\n", prefix, static_ref->ref.internal_ref.offset);
        printf("%s\t}\n", prefix);
    }
    printf("%s}\n", prefix);

}


void verbose_constant_pool_component(cap_file* cf) {

    printf("constant_pool_component {\n");
    if(cf->constant_pool.tag != 0) {
        u2 u2Index = 0;
        printf("\ttag: %u\n", cf->constant_pool.tag);
        printf("\tsize: %u\n", cf->constant_pool.size);
        printf("\tcount: %u\n", cf->constant_pool.count);
        for(; u2Index < cf->constant_pool.count; ++u2Index) {
        printf("\tconstant_pool[%u] {\n", u2Index);
            switch(cf->constant_pool.constant_pool[u2Index].tag) {
                case CF_CONSTANT_CLASSREF:
            printf("\t\ttag: CONSTANT_Classref\n");
            print_classref(&(cf->constant_pool.constant_pool[u2Index].CONSTANT_Classref.class_ref), "\t\t");
            printf("\t\tpadding: %u\n", cf->constant_pool.constant_pool[u2Index].CONSTANT_Classref.padding);
                    break;

                case CF_CONSTANT_INSTANCEFIELDREF:
            printf("\t\ttag: CONSTANT_InstanceFieldref\n");
            print_classref(&(cf->constant_pool.constant_pool[u2Index].CONSTANT_InstanceFieldref.class), "\t\t");
            printf("\t\ttoken: %u\n", cf->constant_pool.constant_pool[u2Index].CONSTANT_InstanceFieldref.token);
                    break;

                case CF_CONSTANT_VIRTUALMETHODREF:
            printf("\t\ttag: CONSTANT_VirtualMethodref\n");
            print_classref(&(cf->constant_pool.constant_pool[u2Index].CONSTANT_VirtualMethodref.class), "\t\t");
            printf("\t\ttoken: %u\n", cf->constant_pool.constant_pool[u2Index].CONSTANT_VirtualMethodref.token);
                    break;

                case CF_CONSTANT_SUPERMETHODREF:
            printf("\t\ttag: CONSTANT_SuperMethodref\n");
            print_classref(&(cf->constant_pool.constant_pool[u2Index].CONSTANT_SuperMethodref.class), "\t\t");
            printf("\t\ttoken: %u\n", cf->constant_pool.constant_pool[u2Index].CONSTANT_SuperMethodref.token);
                    break;

                case CF_CONSTANT_STATICFIELDREF:
            printf("\t\ttag: CONSTANT_StaticFieldref\n");
            print_staticref(&(cf->constant_pool.constant_pool[u2Index].CONSTANT_StaticFieldref.static_field_ref), "field", "\t\t");
                    break;

                case CF_CONSTANT_STATICMETHODREF:
            printf("\t\ttag: CONSTANT_StaticMethodref\n");
            print_staticref(&(cf->constant_pool.constant_pool[u2Index].CONSTANT_StaticMethodref.static_method_ref), "method", "\t\t");

            }
        printf("\t}\n");
        }
    }
    printf("}\n");

}


static void print_type_descriptor(cf_type_descriptor* type_descriptor, const char* prefix) {

    u1 u1Index = 0;
    printf("%soffset: %u\n", prefix, type_descriptor->offset);
    printf("%snibble_count: %u\n", prefix, type_descriptor->nibble_count);
    printf("%stype: ", prefix);
    while(u1Index < type_descriptor->nibble_count) {
        u1 nibble = (u1Index % 2) ? type_descriptor->type[u1Index / 2] & 0x0F : type_descriptor->type[u1Index / 2] >> 4;
        printf("0x%X ", nibble);
        ++u1Index;
    }
    if(type_descriptor->nibble_count % 2)
        printf("0x%X", type_descriptor->type[u1Index / 2] & 0x0F);
    printf("\n");

}

void verbose_class_component(cap_file* cf) {

    printf("class_component {\n");
    if(cf->class.tag != 0) {
        u2 u2Index = 0;
        printf("\ttag: %u\n", cf->class.tag);
        printf("\tsize: %u\n", cf->class.size);
        if(cf->class.can_have_signature_pool) {
        printf("\tsignature_pool_length: %u\n", cf->class.signature_pool_length);
            for(; u2Index < cf->class.signature_pool_count; ++u2Index) {
        printf("\tsignature_pool[%u] {\n", u2Index);
        print_type_descriptor(cf->class.signature_pool + u2Index, "\t\t");
        printf("\t}\n");
            }
        }
        for(u2Index = 0; u2Index < cf->class.interfaces_count; ++u2Index) {
            u1 u1Index = 0;
        printf("\tinterfaces[%u] {\n", u2Index);
            printf("\t\toffset: %u\n", cf->class.interfaces[u2Index].offset);
            printf("\t\tflags:");
            if(cf->class.interfaces[u2Index].flags & CLASS_ACC_INTERFACE)
                printf(" ACC_INTERFACE");
            if(cf->class.interfaces[u2Index].flags & CLASS_ACC_SHAREABLE)
                printf(" ACC_SHAREABLE");
            if(cf->class.interfaces[u2Index].flags & CLASS_ACC_REMOTE)
                printf(" ACC_REMOTE");
            printf("\n");
            printf("\t\tinterface_count: %u\n", cf->class.interfaces[u2Index].interface_count);
            for(; u1Index < cf->class.interfaces[u2Index].interface_count; ++u1Index) {
            printf("\t\tsuperinterfaces[%u] {\n", u1Index);
            print_classref(cf->class.interfaces[u2Index].superinterfaces + u1Index, "\t\t\t");
            printf("\t\t}\n");
            }
            printf("\t\tinterface_name {\n");
            if(cf->class.interfaces[u2Index].has_interface_name) {
                printf("\t\t\tinterface_name_length: %u\n", cf->class.interfaces[u2Index].interface_name.interface_name_length);
                printf("\t\t\tinterface_name: %.*s\n", cf->class.interfaces[u2Index].interface_name.interface_name_length, cf->class.interfaces[u2Index].interface_name.interface_name);
            }
            printf("\t\t}\n");
        printf("\t}\n");
        }
        for(u2Index = 0; u2Index < cf->class.classes_count; ++u2Index) {
            u1 u1Index1 = 0;
        printf("\tclasses[%u] {\n", u2Index);
            printf("\t\toffset: %u\n", cf->class.classes[u2Index].offset);
            printf("\t\tflags:");
            if(cf->class.classes[u2Index].flags & CLASS_ACC_INTERFACE)
                printf(" ACC_INTERFACE");
            if(cf->class.classes[u2Index].flags & CLASS_ACC_SHAREABLE)
                printf(" ACC_SHAREABLE");
            if(cf->class.classes[u2Index].flags & CLASS_ACC_REMOTE)
                printf(" ACC_REMOTE");
            printf("\n");
            printf("\t\tinterface_count: %u\n", cf->class.classes[u2Index].interface_count);
            printf("\t\tsuper_class_ref {\n");
                if(cf->class.classes[u2Index].has_superclass)
                    print_classref(&(cf->class.classes[u2Index].super_class_ref), "\t\t\t");
            printf("\t\t}\n");
            printf("\t\tdeclared_instance_size: %u\n", cf->class.classes[u2Index].declared_instance_size);
            printf("\t\tfirst_reference_token: %u\n", cf->class.classes[u2Index].first_reference_token);
            printf("\t\treference_count: %u\n", cf->class.classes[u2Index].reference_count);
            printf("\t\tpublic_method_table_base: %u\n", cf->class.classes[u2Index].public_method_table_base);
            printf("\t\tpublic_method_table_count: %u\n", cf->class.classes[u2Index].public_method_table_count);
            printf("\t\tpackage_method_table_base: %u\n", cf->class.classes[u2Index].package_method_table_base);
            printf("\t\tpackage_method_table_count: %u\n", cf->class.classes[u2Index].package_method_table_count);
            printf("\t\tpublic_virtual_method_table {\n");
            for(; u1Index1 < cf->class.classes[u2Index].public_method_table_count; ++u1Index1)
            printf("\t\t\t[%u]: %u\n", u1Index1, cf->class.classes[u2Index].public_virtual_method_table[u1Index1]);
            printf("\t\t}\n");
            printf("\t\tpackage_virtual_method_table {\n");
            for(u1Index1 = 0; u1Index1 < cf->class.classes[u2Index].package_method_table_count; ++u1Index1)
            printf("\t\t\t[%u]: %u\n", u1Index1, cf->class.classes[u2Index].package_virtual_method_table[u1Index1]);
            printf("\t\t}\n");
            for(u1Index1 = 0; u1Index1 < cf->class.classes[u2Index].interface_count; ++u1Index1) {
                u1 u1Index2 = 0;
            printf("\t\tinterfaces[%u] {\n", u1Index1);
                printf("\t\t\tinterface {\n");
                    print_classref(&(cf->class.classes[u2Index].interfaces[u1Index1].interface), "\t\t\t\t");
                printf("\t\t\t}\n");
                printf("\t\t\tcount: %u\n", cf->class.classes[u2Index].interfaces[u1Index1].count);
                printf("\t\t\tindex {\n");
                for(; u1Index2 < cf->class.classes[u2Index].interfaces[u1Index1].count; ++u1Index2)
                printf("\t\t\t\t[%u]: %u\n", u1Index2, cf->class.classes[u2Index].interfaces[u1Index1].index[u1Index2]);
                printf("\t\t\t}\n");
            printf("\t\t}\n");
            }
            printf("\t\tremote_interfaces {\n");
            if(cf->class.classes[u2Index].has_remote_interfaces) {
                printf("\t\t\tremote_methods_count: %u\n", cf->class.classes[u2Index].remote_interfaces.remote_methods_count);
                for(u1Index1 = 0; u1Index1 < cf->class.classes[u2Index].remote_interfaces.remote_methods_count; ++u1Index1) {
                printf("\t\t\tremote_methods[%u] {\n", u1Index1);
                    printf("\t\t\t\tremote_method_hash: %u\n", cf->class.classes[u2Index].remote_interfaces.remote_methods[u1Index1].remote_method_hash);
                    printf("\t\t\t\tsignature_offset: %u\n", cf->class.classes[u2Index].remote_interfaces.remote_methods[u1Index1].signature_offset);
                    printf("\t\t\t\tvirtual_method_token: %u\n", cf->class.classes[u2Index].remote_interfaces.remote_methods[u1Index1].virtual_method_token);
                printf("\t\t\t}\n");
                }
                printf("\t\t\thash_modifier_length: %u\n", cf->class.classes[u2Index].remote_interfaces.hash_modifier_length);
                printf("\t\t\thash_modifier {\n");
                for(u1Index1 = 0; u1Index1 < cf->class.classes[u2Index].remote_interfaces.hash_modifier_length; ++u1Index1)
                printf("\t\t\t\t[%u]: %u\n", u1Index1, cf->class.classes[u2Index].remote_interfaces.hash_modifier[u1Index1]);
                printf("\t\t\t}\n");
                printf("\t\t\tclass_name_length: %u\n", cf->class.classes[u2Index].remote_interfaces.class_name_length);
                printf("\t\t\tclass_name: %.*s\n", cf->class.classes[u2Index].remote_interfaces.class_name_length, cf->class.classes[u2Index].remote_interfaces.class_name);
                printf("\t\t\tremote_interfaces_count: %u\n", cf->class.classes[u2Index].remote_interfaces.remote_interfaces_count);
                for(u1Index1 = 0; u1Index1 < cf->class.classes[u2Index].remote_interfaces.remote_interfaces_count; ++u1Index1) {
                printf("\t\t\tremote_interfaces[%u] {\n", u1Index1);
                    print_classref(cf->class.classes[u2Index].remote_interfaces.remote_interfaces + u1Index1, "\t\t\t\t");
                printf("\t\t\t}\n");
                }
            }
            printf("\t\t}\n");
        printf("\t}\n");
        }
    }
    printf("}\n");

}

void verbose_method_component(cap_file* cf) {

    printf("method_component {\n");
    if(cf->method.tag != 0) {
        u1 u1Index = 0;
        u2 u2Index1 = 0;
        printf("\ttag: %u\n", cf->method.tag);
        printf("\tsize: %u\n", cf->method.size);
        printf("\thandler_count: %u\n", cf->method.handler_count);
        for(; u1Index < cf->method.handler_count; ++u1Index) {
        printf("\texception_handlers[%u] {\n", u1Index);
            printf("\t\tstart_offset: %u\n", cf->method.exception_handlers[u1Index].start_offset);
            printf("\t\tstop_bit: %u\n", cf->method.exception_handlers[u1Index].stop_bit);
            printf("\t\tactive_length: %u\n", cf->method.exception_handlers[u1Index].active_length);
            printf("\t\thandler_offset: %u\n", cf->method.exception_handlers[u1Index].handler_offset);
            printf("\t\tcatch_type_index: %u\n", cf->method.exception_handlers[u1Index].catch_type_index);
        printf("\t}\n");
        }
        for(; u2Index1 < cf->method.method_count; ++u2Index1) {
            u2 u2Index2 = 0;
        printf("\tmethods[%u] {\n", u2Index1);
            printf("\t\toffset: %u\n", cf->method.methods[u2Index1].offset);
            if(cf->method.methods[u2Index1].method_header.flags & METHOD_ACC_EXTENDED) {
            printf("\t\textended_method_header {\n");
                printf("\t\t\tflags: ACC_EXTENDED");
                if(cf->method.methods[u2Index1].method_header.flags & METHOD_ACC_ABSTRACT)
                printf(" ACC_ABSTRACT");
                printf("\n");
                printf("\t\t\tpadding: %u\n", cf->method.methods[u2Index1].method_header.extended_method_header.padding);
                printf("\t\t\tmax_stack: %u\n", cf->method.methods[u2Index1].method_header.extended_method_header.max_stack);
                printf("\t\t\tnargs: %u\n", cf->method.methods[u2Index1].method_header.extended_method_header.nargs);
                printf("\t\t\tmax_locals: %u\n", cf->method.methods[u2Index1].method_header.extended_method_header.max_locals);
            printf("\t\t}\n");
            } else {
            printf("\t\tmethod_header {\n");
                printf("\t\t\tflags:");
                if(cf->method.methods[u2Index1].method_header.flags & METHOD_ACC_ABSTRACT)
                printf(" ACC_ABSTRACT");
                printf("\n");
                printf("\t\t\tmax_stack: %u\n", cf->method.methods[u2Index1].method_header.standard_method_header.max_stack);
                printf("\t\t\tnargs: %u\n", cf->method.methods[u2Index1].method_header.standard_method_header.nargs);
                printf("\t\t\tmax_locals: %u\n", cf->method.methods[u2Index1].method_header.standard_method_header.max_locals);
            printf("\t\t}\n");
            }
            printf("\t\tbytecodes {\n");
            for(; u2Index2 < cf->method.methods[u2Index1].bytecode_count; ++u2Index2)
                printf("%u ", cf->method.methods[u2Index1].bytecodes[u2Index2]);
            printf("\n\t\t}\n");
        printf("\t}\n");
        }
    }
    printf("}\n");

}


void verbose_static_field_component(cap_file* cf) {

    printf("static_field_component {\n");
    if(cf->static_field.tag != 0) {
        u2 u2Index1 = 0;
        printf("\ttag: %u\n", cf->static_field.tag);
        printf("\tsize: %u\n", cf->static_field.size);
        printf("\timage_size: %u\n", cf->static_field.image_size);
        printf("\treference_count: %u\n", cf->static_field.reference_count);
        printf("\tarray_init_count: %u\n", cf->static_field.array_init_count);
        for(; u2Index1 < cf->static_field.array_init_count; ++u2Index1) {
            u2 u2Index2 = 0;
        printf("\tarray_init[%u] {\n", u2Index1);
            printf("\t\ttype: ");
            switch(cf->static_field.array_init[u2Index1].type) {
                case 2:
                    printf("boolean\n");
                    break;
                case 3:
                    printf("byte\n");
                    break;
                case 4:
                    printf("short\n");
                    break;
                case 5:
                    printf("int\n");
            }
            printf("\t\tcount: %u\n", cf->static_field.array_init[u2Index1].count);
            printf("\t\tvalues {\n");
            for(; u2Index2 < cf->static_field.array_init[u2Index1].count; ++u2Index2)
                printf("\t\t\t[%u]: %u\n", u2Index2, cf->static_field.array_init[u2Index1].values[u2Index2]);
            printf("\t\t}\n");
        printf("\t}\n");
        }
        printf("\tdefault_value_count: %u\n", cf->static_field.default_value_count);
        printf("\tnon_default_value_count: %u\n", cf->static_field.non_default_value_count);
        printf("\tnon_default_values {\n");
        for(u2Index1 = 0; u2Index1 < cf->static_field.non_default_value_count; ++u2Index1)
            printf("\t\t[%u]: %u\n", u2Index1, cf->static_field.non_default_values[u2Index1]);
        printf("\t}\n");
    }
    printf("}\n");

}


void verbose_reference_location_component(cap_file* cf) {

    printf("reference_location_component {\n");
    if(cf->reference_location.tag != 0) {
        u2 u2Index = 0;
        printf("\ttag: %u\n", cf->reference_location.tag);
        printf("\tsize: %u\n", cf->reference_location.size);
        printf("\tbyte_index_count: %u\n", cf->reference_location.byte_index_count);
        printf("\toffsets_to_byte_indices {\n");
        for(; u2Index < cf->reference_location.byte_index_count; ++u2Index)
            printf("\t\t[%u]: %u\n", u2Index, cf->reference_location.offset_to_byte_indices[u2Index]);
        printf("\t}\n");
        printf("\tbyte2_index_count: %u\n", cf->reference_location.byte2_index_count);
        printf("\toffsets_to_byte2_indices {\n");
        for(u2Index = 0; u2Index < cf->reference_location.byte2_index_count; ++u2Index)
            printf("\t\t[%u]: %u\n", u2Index, cf->reference_location.offset_to_byte2_indices[u2Index]);
        printf("\t}\n");
    }
    printf("}\n");

}



void verbose_export_component(cap_file* cf) {

    printf("export_component {\n");
    if(cf->export.tag != 0) {
        u1 u1Index1 = 0;
        printf("\ttag: %u\n", cf->export.tag);
        printf("\tsize: %u\n", cf->export.size);
        printf("\tclass_count: %u\n", cf->export.class_count);
        for(; u1Index1 < cf->export.class_count; ++u1Index1) {
            u1 u1Index2 = 0;
        printf("\tclass_exports[%u] {\n", u1Index1);
            printf("\t\tclass_offset: %u\n", cf->export.class_exports[u1Index1].class_offset);
            printf("\t\tstatic_field_count: %u\n", cf->export.class_exports[u1Index1].static_field_count);
            printf("\t\tstatic_method_count: %u\n", cf->export.class_exports[u1Index1].static_method_count);
            printf("\t\tstatic_field_offsets {\n");
            for(; u1Index2 < cf->export.class_exports[u1Index1].static_field_count; ++u1Index2)
                printf("\t\t\t[%u]: %u\n", u1Index2, cf->export.class_exports[u1Index1].static_field_offsets[u1Index2]);
            printf("\t\t}\n");
            printf("\t\tstatic_method_offsets {\n");
            for(u1Index2 = 0; u1Index2 < cf->export.class_exports[u1Index1].static_method_count; ++u1Index2)
                printf("\t\t\t[%u]: %u\n", u1Index2, cf->export.class_exports[u1Index1].static_method_offsets[u1Index2]);
            printf("\t\t}\n");
        printf("\t}\n");
        }
    }
    printf("}\n");

}



void verbose_descriptor_component(cap_file* cf) {

    printf("descriptor_component {\n");
    if(cf->descriptor.tag != 0) {
        u1 u1Index1 = 0;
        u2 u2Index = 0;
        printf("\ttag: %u\n", cf->descriptor.tag);
        printf("\tsize: %u\n", cf->descriptor.size);
        printf("\tclass_count: %u\n", cf->descriptor.class_count);
        for(; u1Index1 < cf->descriptor.class_count; ++u1Index1) {
            u1 u1Index2 = 0;
        printf("\tclasses[%u] {\n", u1Index1);
            printf("\t\ttoken: %u\n", cf->descriptor.classes[u1Index1].token);
            printf("\t\taccess_flags:");
            if(cf->descriptor.classes[u1Index1].access_flags & DESCRIPTOR_ACC_PUBLIC)
                printf(" ACC_PUBLIC");
            if(cf->descriptor.classes[u1Index1].access_flags & DESCRIPTOR_ACC_FINAL)
                printf(" ACC_FINAL");
            if(cf->descriptor.classes[u1Index1].access_flags & DESCRIPTOR_ACC_INTERFACE)
                printf(" ACC_INTERFACE");
            if(cf->descriptor.classes[u1Index1].access_flags & DESCRIPTOR_ACC_ABSTRACT1)
                printf(" ACC_ABSTRACT");
            printf("\n");
            printf("\t\tthis_class_ref {\n");
            print_classref(&(cf->descriptor.classes[u1Index1].this_class_ref), "\t\t\t");
            printf("\t\t}\n");
            printf("\t\tinterface_count: %u\n", cf->descriptor.classes[u1Index1].interface_count);
            printf("\t\tfield_count: %u\n", cf->descriptor.classes[u1Index1].field_count);
            printf("\t\tmethod_count: %u\n", cf->descriptor.classes[u1Index1].method_count);
            for(; u1Index2 < cf->descriptor.classes[u1Index1].interface_count; ++u1Index2) {
            printf("\t\tinterfaces[%u] {\n", u1Index2);
            print_classref(cf->descriptor.classes[u1Index1].interfaces + u1Index2, "\t\t\t");
            printf("\t\t}\n");
            }
            for(u2Index = 0; u2Index < cf->descriptor.classes[u1Index1].field_count; ++u2Index) {
            printf("\t\tfields[%u] {\n", u2Index);
                printf("\t\t\ttoken: %u\n", cf->descriptor.classes[u1Index1].fields[u2Index].token);
                printf("\t\t\taccess_flags:");
                if(cf->descriptor.classes[u1Index1].fields[u2Index].access_flags & DESCRIPTOR_ACC_PUBLIC)
                    printf(" ACC_PUBLIC");
                if(cf->descriptor.classes[u1Index1].fields[u2Index].access_flags & DESCRIPTOR_ACC_PRIVATE)
                    printf(" ACC_PRIVATE");
                if(cf->descriptor.classes[u1Index1].fields[u2Index].access_flags & DESCRIPTOR_ACC_PROTECTED)
                    printf(" ACC_PROTECTED");
                if(cf->descriptor.classes[u1Index1].fields[u2Index].access_flags & DESCRIPTOR_ACC_STATIC)
                    printf(" ACC_STATIC");
                if(cf->descriptor.classes[u1Index1].fields[u2Index].access_flags & DESCRIPTOR_ACC_FINAL)
                    printf(" ACC_FINAL");
                printf("\n");
                printf("\t\t\tfield_ref {\n");
                if(cf->descriptor.classes[u1Index1].fields[u2Index].access_flags & DESCRIPTOR_ACC_STATIC) {
                    print_staticref(&(cf->descriptor.classes[u1Index1].fields[u2Index].field_ref.static_field), "field", "\t\t\t\t");
                } else {
                    print_classref(&(cf->descriptor.classes[u1Index1].fields[u2Index].field_ref.instance_field.class_ref), "\t\t\t\t");
                    printf("\t\t\t\ttoken: %u\n", cf->descriptor.classes[u1Index1].fields[u2Index].field_ref.instance_field.token);
                }
                printf("\t\t\t}\n");
                printf("\t\t\ttype {\n");
                switch(cf->descriptor.classes[u1Index1].fields[u2Index].type.primitive_type) {
                    case 0x8002:
                    printf("\t\t\t\tprimitive_type: boolean\n");
                        break;

                    case 0x8003:
                    printf("\t\t\t\tprimitive_type: byte\n");
                        break;

                    case 0x8004:
                    printf("\t\t\t\tprimitive_type: short\n");
                        break;

                    case 0x8005:
                    printf("\t\t\t\tprimitive_type: int\n");
                        break;
    
                    default:
                    printf("\t\t\t\treference_type: %u\n", cf->descriptor.classes[u1Index1].fields[u2Index].type.reference_type);
                }
                printf("\t\t\t}\n");
            printf("\t\t}\n");
            }
            for(u2Index = 0; u2Index < cf->descriptor.classes[u1Index1].method_count; ++u2Index) {
            printf("\t\tmethods[%u] {\n", u2Index);
                printf("\t\t\ttoken: %u\n", cf->descriptor.classes[u1Index1].methods[u2Index].token);
                printf("\t\t\taccess_flags:");
                if(cf->descriptor.classes[u1Index1].methods[u2Index].access_flags & DESCRIPTOR_ACC_PUBLIC)
                    printf(" ACC_PUBLIC");
                if(cf->descriptor.classes[u1Index1].methods[u2Index].access_flags & DESCRIPTOR_ACC_PRIVATE)
                    printf(" ACC_PRIVATE");
                if(cf->descriptor.classes[u1Index1].methods[u2Index].access_flags & DESCRIPTOR_ACC_PROTECTED)
                    printf(" ACC_PROTECTED");
                if(cf->descriptor.classes[u1Index1].methods[u2Index].access_flags & DESCRIPTOR_ACC_STATIC)
                    printf(" ACC_STATIC");
                if(cf->descriptor.classes[u1Index1].methods[u2Index].access_flags & DESCRIPTOR_ACC_FINAL)
                    printf(" ACC_FINAL");
                if(cf->descriptor.classes[u1Index1].methods[u2Index].access_flags & DESCRIPTOR_ACC_ABSTRACT2)
                    printf(" ACC_ABSTRACT");
                if(cf->descriptor.classes[u1Index1].methods[u2Index].access_flags & DESCRIPTOR_ACC_INIT)
                    printf(" ACC_INIT");
                printf("\n");
                printf("\t\t\tmethod_offset: %u\n", cf->descriptor.classes[u1Index1].methods[u2Index].method_offset);
                printf("\t\t\ttype_offset: %u\n", cf->descriptor.classes[u1Index1].methods[u2Index].type_offset);
                printf("\t\t\tbytecode_count: %u\n", cf->descriptor.classes[u1Index1].methods[u2Index].bytecode_count);
                printf("\t\t\texception_handler_count: %u\n", cf->descriptor.classes[u1Index1].methods[u2Index].exception_handler_count);
                printf("\t\t\texception_handler_index: %u\n", cf->descriptor.classes[u1Index1].methods[u2Index].exception_handler_index);
            printf("\t\t}\n");
            }
        printf("\t}\n");
        }
        printf("\ttypes {\n");
            printf("\t\tconstant_pool_count: %u\n", cf->descriptor.types.constant_pool_count);
            printf("\t\tconstant_pool_types {\n");
            for(u2Index = 0; u2Index < cf->descriptor.types.constant_pool_count; ++u2Index)
                printf("\t\t\t[%u]: %u\n", u2Index, cf->descriptor.types.constant_pool_types[u2Index]);
            printf("\t\t}\n");
            for(u2Index = 0; u2Index < cf->descriptor.types.type_desc_count; ++u2Index) {
            printf("\t\ttype_desc[%u] {\n", u2Index);
                print_type_descriptor(cf->descriptor.types.type_desc + u2Index, "\t\t\t");
            printf("\t\t}\n");
            }
        printf("\t}\n");
    }
    printf("}\n");

}
