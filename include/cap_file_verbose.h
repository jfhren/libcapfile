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
 * \file cap_file_verbose.h
 * \brief Output a human readble version of a CAP file in its straightforward
 * representation.  
 */

#include "cap_file.h"

/**
 * \brief Output the manifest of a CAP file on the standard output.
 *
 * \param cf The CAP file to output.
 */
void verbose_manifest(cap_file* cf);

/**
 * \brief Output the Header component of a CAP file on the standard output.
 *
 * \param cf The CAP file to output.
 */
void verbose_header_component(cap_file* cf);

/**
 * \brief Output the Directory component of a CAP file on the standard output.
 *
 * \param cf The CAP file to output.
 */
void verbose_directory_component(cap_file* cf);

/**
 * \brief Output the Applet component of a CAP file on the standard output.
 *
 * \param cf The CAP file to output.
 */
void verbose_applet_component(cap_file* cf);

/**
 * \brief Output the Import component of a CAP file on the standard output.
 *
 * \param cf The CAP file to output.
 */
void verbose_import_component(cap_file* cf);

/**
 * \brief Output the Constant Pool component of a CAP file on the standard output.
 *
 * \param cf The CAP file to output.
 */
void verbose_constant_pool_component(cap_file* cf);

/**
 * \brief Output the Class component of a CAP file on the standard output.
 *
 * \param cf The CAP file to output.
 */
void verbose_class_component(cap_file* cf);

/**
 * \brief Output the Method component of a CAP file on the standard output.
 *
 * \param cf The CAP file to output.
 */
void verbose_method_component(cap_file* cf);

/**
 * \brief Output the Static Field component of a CAP file on the standard output.
 *
 * \param cf The CAP file to output.
 */
void verbose_static_field_component(cap_file* cf);

/**
 * \brief Output the Reference Location component of a CAP file on the standard output.
 *
 * \param cf The CAP file to output.
 */
void verbose_reference_location_component(cap_file* cf);

/**
 * \brief Output the Export component of a CAP file on the standard output.
 *
 * \param cf The CAP file to output.
 */
void verbose_export_component(cap_file* cf);

/**
 * \brief Output the Descriptor component of a CAP file on the standard output.
 *
 * \param cf The CAP file to output.
 */
void verbose_descriptor_component(cap_file* cf);
