/**
 *Licensed to the Apache Software Foundation (ASF) under one
 *or more contributor license agreements.  See the NOTICE file
 *distributed with this work for additional information
 *regarding copyright ownership.  The ASF licenses this file
 *to you under the Apache License, Version 2.0 (the
 *"License"); you may not use this file except in compliance
 *with the License.  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *Unless required by applicable law or agreed to in writing,
 *software distributed under the License is distributed on an
 *"AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 *specific language governing permissions and limitations
 *under the License.
 */
#ifndef __DYN_INTERFACE_H_
#define __DYN_INTERFACE_H_

#include "dyn_common.h"
#include "dyn_type.h"
#include "dyn_function.h"
#include "dfi_log_util.h"

#include "version.h"

DFI_SETUP_LOG_HEADER(dynInterface);

/* Description string
 *
 * Descriptor (interface) = HeaderSection AnnotationSection TypesSection MethodsSection
 *
 * HeaderSection=
 * ':header\n' [NameValue]*
 * ':annotations\n' [NameValue]*
 * ':types\n' [TypeIdValue]*
 * ':methods\n' [MethodIdValue]
 *
 */
typedef struct _dyn_interface_type dyn_interface_type;

TAILQ_HEAD(methods_head, method_entry);
struct method_entry {
    int index;
    char *id;
    char *name;
    dyn_function_type *dynFunc;

    TAILQ_ENTRY(method_entry) entries; 
};

int dynInterface_parse(FILE *descriptor, dyn_interface_type **out);
void dynInterface_destroy(dyn_interface_type *intf);

int dynInterface_getName(dyn_interface_type *intf, char **name);
int dynInterface_getVersion(dyn_interface_type *intf, version_pt* version);
int dynInterface_getVersionString(dyn_interface_type *intf, char **version);
int dynInterface_getHeaderEntry(dyn_interface_type *intf, const char *name, char **value);
int dynInterface_getAnnotationEntry(dyn_interface_type *intf, const char *name, char **value);
int dynInterface_methods(dyn_interface_type *intf, struct methods_head **list);
int dynInterface_nrOfMethods(dyn_interface_type *intf);


#endif
