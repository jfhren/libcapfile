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
 * \file cap_file_analyze.h
 * \brief Convert a straightforward representation of CAP file into an analyzed one.
 */

#ifndef CAP_FILE_ANALYZE_H
#define CAP_FILE_ANALYZE_H

#include "analyzed_cap_file.h"
#include "cap_file.h"
#include "exp_file.h"

/**
 * \brief Recursively search in the given directory for export files and build
 *        an array of parsed export files.
 *
 * \param directory       The root directory in which the search starts.
 * \param export_files    The built array of parsed export files (might be not
 *                        empty).
 * \param nb_export_files The number of found and parsed export files (might be
 *                        not 0).
 *
 * \return Return -1 if an error occurred, 0 else.
 */
export_file** get_export_files_from_directories(char* const* directories, int nb_directories, int* nb_export_files);

/**
 * \brief Analyze a straightforward representation of a CAP file into a more
 *        useful format.
 *
 * \param cf              The straightforward representation of the CAP file.
 * \param export_files    An array of parsed export files.
 * \param nb_export_files The number of parsed export files in the array.
 *
 * \return Return the analyzed CAP file or NULL if an error occurred.
 */
analyzed_cap_file* analyze_cap_file(cap_file* cf, export_file** export_files, int nb_export_files);

#endif
