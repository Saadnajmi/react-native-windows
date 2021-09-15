/**
 * Copyright (c) Microsoft Corporation.
 * Licensed under the MIT License.
 * @format
 */

'use strict';

import {
  NativeModuleFunctionTypeAnnotation,
  NativeModulePropertyShape,
  SchemaType,
} from 'react-native-tscodegen';
import {AliasMap, setPreferredModuleName} from './AliasManaging';
import {createAliasMap, generateAliases} from './AliasGen';
import {translateArgs, translateSpecArgs} from './ParamTypes';
import {translateImplReturnType, translateSpecReturnType} from './ReturnTypes';

type FilesOutput = Map<string, string>;

const moduleTemplate = `
/*
 * This file is auto-generated from a NativeModule spec file in js.
 *
 * This is a C++ Spec class that should be used with MakeTurboModuleProvider to register native modules
 * in a way that also verifies at compile time that the native module matches the interface required
 * by the TurboModule JS spec.
 */
#pragma once

#include "NativeModules.h"
#include <tuple>

namespace ::_NAMESPACE_:: {
::_MODULE_ALIASED_STRUCTS_::
struct ::_MODULE_NAME_::Spec : winrt::Microsoft::ReactNative::TurboModuleSpec {
  static constexpr auto methods = std::tuple{
::_MODULE_PROPERTIES_TUPLE_::
  };

  template <class TModule>
  static constexpr void ValidateModule() noexcept {
    constexpr auto methodCheckResults = CheckMethods<TModule, ::_MODULE_NAME_::Spec>();

::_MODULE_PROPERTIES_SPEC_ERRORS_::
  }
};

} // namespace ::_NAMESPACE_::
`;

function isMethodSync(funcType: NativeModuleFunctionTypeAnnotation) {
  return (
    funcType.returnTypeAnnotation.type !== 'VoidTypeAnnotation' &&
    funcType.returnTypeAnnotation.type !== 'PromiseTypeAnnotation'
  );
}

function isMethodReturnPromise(funcType: NativeModuleFunctionTypeAnnotation) {
  return funcType.returnTypeAnnotation.type === 'PromiseTypeAnnotation';
}

function getPossibleMethodSignatures(
  prop: NativeModulePropertyShape,
  funcType: NativeModuleFunctionTypeAnnotation,
  aliases: AliasMap,
  baseAliasName: string,
): string[] {
  const args = translateArgs(funcType.params, aliases, baseAliasName);
  if (isMethodReturnPromise(funcType)) {
    // TODO: type of the promise could be provided in the future
    args.push('React::ReactPromise<React::JSValue> &&result');
  }

  // TODO: be much more exhastive on the possible method signatures that can be used..
  const sig = `REACT_${isMethodSync(funcType) ? 'SYNC_' : ''}METHOD(${
    prop.name
  }) ${translateImplReturnType(
    funcType.returnTypeAnnotation,
    aliases,
    baseAliasName,
  )} ${prop.name}(${args.join(', ')}) noexcept { /* implementation */ }}`;

  const staticsig = `REACT_${isMethodSync(funcType) ? 'SYNC_' : ''}METHOD(${
    prop.name
  }) static ${translateImplReturnType(
    funcType.returnTypeAnnotation,
    aliases,
    baseAliasName,
  )} ${prop.name}(${args.join(', ')}) noexcept { /* implementation */ }}`;

  return [sig, staticsig];
}

function translatePossibleMethodSignatures(
  prop: NativeModulePropertyShape,
  funcType: NativeModuleFunctionTypeAnnotation,
  aliases: AliasMap,
  baseAliasName: string,
): string {
  return getPossibleMethodSignatures(prop, funcType, aliases, baseAliasName)
    .map(sig => `"    ${sig}\\n"`)
    .join('\n          ');
}

function renderProperties(
  properties: ReadonlyArray<NativeModulePropertyShape>,
  aliases: AliasMap,
  tuple: boolean,
): string {
  // TODO: generate code for constants
  return properties
    .filter(prop => prop.name !== 'getConstants')
    .map((prop, index) => {
      // TODO: prop.optional === true
      // TODO: prop.typeAnnotation.type === 'NullableTypeAnnotation'
      const propAliasName = prop.name;
      const funcType =
        prop.typeAnnotation.type === 'NullableTypeAnnotation'
          ? prop.typeAnnotation.typeAnnotation
          : prop.typeAnnotation;

      const traversedArgs = translateSpecArgs(
        funcType.params,
        aliases,
        propAliasName,
      );

      const translatedReturnParam = translateSpecReturnType(
        funcType.returnTypeAnnotation,
        aliases,
        propAliasName,
      );

      if (isMethodReturnPromise(funcType)) {
        // TODO: type of the promise could be provided in the future
        traversedArgs.push('Promise<React::JSValue>');
      }

      if (tuple) {
        return `      ${
          isMethodSync(funcType) ? 'Sync' : ''
        }Method<${translatedReturnParam}(${traversedArgs.join(
          ', ',
        )}) noexcept>{${index}, L"${prop.name}"},`;
      } else {
        return `    REACT_SHOW_METHOD_SPEC_ERRORS(
          ${index},
          "${prop.name}",
          ${translatePossibleMethodSignatures(
            prop,
            funcType,
            aliases,
            propAliasName,
          )});`;
      }
    })
    .join('\n');
}

export function createNM2Generator({namespace}: {namespace: string}) {
  return (
    _libraryName: string,
    schema: SchemaType,
    _moduleSpecName: string,
  ): FilesOutput => {
    const files = new Map<string, string>();

    for (const moduleName of Object.keys(schema.modules)) {
      const nativeModule = schema.modules[moduleName];
      // from 0.65 facebook's react-native-codegen
      // the module name has the Native prefix comparing to 0.63
      // when reading files we provided
      const preferredModuleName = moduleName.startsWith('Native')
        ? moduleName.substr(6)
        : moduleName;
      setPreferredModuleName(preferredModuleName);

      if (nativeModule.type === 'NativeModule') {
        console.log(`Generating Native${preferredModuleName}Spec.g.h`);

        // copy all explicit to a map
        const aliases: AliasMap = createAliasMap(nativeModule.aliases);

        // prepare members for turbo modules
        const properties = nativeModule.spec.properties;
        const traversedProperties = renderProperties(
          properties,
          aliases,
          false,
        );
        const traversedPropertyTuples = renderProperties(
          properties,
          aliases,
          true,
        );

        // generate code for structs
        const traversedAliasedStructs = generateAliases(aliases);

        files.set(
          `Native${preferredModuleName}Spec.g.h`,
          moduleTemplate
            .replace(/::_MODULE_ALIASED_STRUCTS_::/g, traversedAliasedStructs)
            .replace(/::_MODULE_PROPERTIES_TUPLE_::/g, traversedPropertyTuples)
            .replace(
              /::_MODULE_PROPERTIES_SPEC_ERRORS_::/g,
              traversedProperties,
            )
            .replace(/::_MODULE_NAME_::/g, preferredModuleName)
            .replace(/::_NAMESPACE_::/g, namespace),
        );
      }
    }

    return files;
  };
}