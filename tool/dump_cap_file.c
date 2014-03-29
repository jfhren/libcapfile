/*
 * Copyright Inria:
 * Jean-François Hren
 * 
 * jfhren[at]gmail[dot]com
 * michael[dot]hauspie[at]lifl[dot]com
 * 
 * This software is a computer program whose purpose is to read, analyze,
 * modify, generate and write Java Card 2 .CAP file.
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
 * \file dump_cap_file.c
 * \brief Read, parse and output a .CAP file.
 */

#include <stdlib.h>
#include <stdio.h>

#include <cap_file.h>
#include <cap_file_reader.h>
#include <cap_file_verbose.h>

int main(int argc, char* argv[]) {

    cap_file* cf = NULL;

    if(argc != 2) {
        fprintf(stderr, "Usage: %s capFile\n", argv[0]);
        return EXIT_FAILURE;
    }

    if((cf = read_cap_file(argv[1])) == NULL)
        return EXIT_FAILURE;

    verbose_manifest(cf);
    printf("\n");
    verbose_header_component(cf);
    printf("\n");
    verbose_directory_component(cf);
    printf("\n");
    verbose_applet_component(cf);
    printf("\n");
    verbose_import_component(cf);
    printf("\n");
    verbose_constant_pool_component(cf);
    printf("\n");
    verbose_class_component(cf);
    printf("\n");
    verbose_method_component(cf);
    printf("\n");
    verbose_static_field_component(cf); 
    printf("\n");
    verbose_reference_location_component(cf);
    printf("\n");
    verbose_export_component(cf);
    printf("\n");
    verbose_descriptor_component(cf);
    return EXIT_SUCCESS;

}    
