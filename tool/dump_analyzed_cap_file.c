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
 * \file dump_analyzed_cap_file.c
 * \brief Read, parse, analyze and output a .CAP file.
 */

#include <stdlib.h>
#include <stdio.h>

#include <exp_file.h>
#include <cap_file.h>
#include <cap_file_reader.h>
#include <analyzed_cap_file.h>
#include <cap_file_analyze.h>
#include <analyzed_cap_file_verbose.h>


int main(int argc, char* argv[]) {

    cap_file* cf = NULL;
    analyzed_cap_file* acf = NULL;

    int i = 0;
    char** directories = NULL;
    int nb_directories = 0;
    export_file** export_files = NULL;
    int nb_export_files = 0;

    if(argc < 3) {
        fprintf(stderr, "Usage: %s exp_files_directory [exp_files_directory] filename\n", argv[0]);
        return EXIT_FAILURE;
    }

    if((cf = read_cap_file(argv[argc-1])) == NULL)
        return EXIT_FAILURE;

    nb_directories = argc - 2;
    directories = (char**)malloc(sizeof(char*) * nb_directories);
    if(directories == NULL) {
        perror("main");
        return EXIT_FAILURE;
    }

    for(; i < nb_directories; ++i)
        directories[i] = argv[1 + i];

    export_files = get_export_files_from_directories(directories, nb_directories, &nb_export_files);

    if((acf = analyze_cap_file(cf, export_files, nb_export_files)) == NULL)
        return EXIT_FAILURE;

    verbose_constant_info(acf);
    printf("\n");
    verbose_imported_package(acf);
    printf("\n");
    verbose_constant_pool(acf);
    printf("\n");
    verbose_signature_pool(acf);
    printf("\n");
    verbose_interfaces(acf);
    printf("\n");
    verbose_classes(acf);
    printf("\n");
    verbose_exception_handlers(acf);

    return EXIT_SUCCESS;

}
