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
 * \file analyzed_cap_file_verbose.h
 * \brief Output a human readable version of an analyzed CAP file.
 */

#ifndef ANALYZED_CAP_FILE_VERBOSE_H
#define ANALYZED_CAP_FILE_VERBOSE_H

#include "analyzed_cap_file.h"

/**
 * \brief Output the constant info of an analyzed CAP file on the standard
 * output.
 *
 * \param acf The analyzed CAP file to output.
 */
void verbose_constant_info(analyzed_cap_file* acf);

/**
 * \brief Output the imported packages of an analyzed CAP file on the standard
 * output.
 *
 * \param acf The analyzed CAP file to output.
 */
void verbose_imported_package(analyzed_cap_file* acf);

/**
 * \brief Output the constant pool of an analyzed CAP file on the standard
 * output.
 *
 * \param acf The analyzed CAP file to output.
 */
void verbose_constant_pool(analyzed_cap_file* acf);

/**
 * \brief Output the signature pool of an analyzed CAP file on the standard
 * output.
 *
 * \param acf The analyzed CAP file to output.
 */
void verbose_signature_pool(analyzed_cap_file* acf);

/**
 * \brief Output the interfaces and their methods of an analyzed CAP file on
 * the standard output.
 *
 * \param acf The analyzed CAP file to output.
 */
void verbose_interfaces(analyzed_cap_file* acf);

/**
 * \brief Output the classes and their fields and methods of an analyzed CAP
 * file on the standard output.
 *
 * \param acf The analyzed CAP file to output.
 */
void verbose_classes(analyzed_cap_file* acf);

/**
 * \brief Output the exception handlers of an analyzed CAP file on the standard
 * output.
 *
 * \param acf The analyzed CAP file to output.
 */
void verbose_exception_handlers(analyzed_cap_file* acf);

#endif
