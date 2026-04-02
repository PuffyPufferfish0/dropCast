set(CMAKE_CXX_COMPILER "/nix/store/zx71vq7s1v840wqsrw2m2ckmxn413a2b-gcc-wrapper-13.3.0/bin/g++")
set(CMAKE_CXX_COMPILER_ARG1 "")
set(CMAKE_CXX_COMPILER_ID "GNU")
set(CMAKE_CXX_COMPILER_VERSION "13.3.0")
set(CMAKE_CXX_COMPILER_VERSION_INTERNAL "")
set(CMAKE_CXX_COMPILER_WRAPPER "")
set(CMAKE_CXX_STANDARD_COMPUTED_DEFAULT "17")
set(CMAKE_CXX_EXTENSIONS_COMPUTED_DEFAULT "ON")
set(CMAKE_CXX_STANDARD_LATEST "23")
set(CMAKE_CXX_COMPILE_FEATURES "cxx_std_98;cxx_template_template_parameters;cxx_std_11;cxx_alias_templates;cxx_alignas;cxx_alignof;cxx_attributes;cxx_auto_type;cxx_constexpr;cxx_decltype;cxx_decltype_incomplete_return_types;cxx_default_function_template_args;cxx_defaulted_functions;cxx_defaulted_move_initializers;cxx_delegating_constructors;cxx_deleted_functions;cxx_enum_forward_declarations;cxx_explicit_conversions;cxx_extended_friend_declarations;cxx_extern_templates;cxx_final;cxx_func_identifier;cxx_generalized_initializers;cxx_inheriting_constructors;cxx_inline_namespaces;cxx_lambdas;cxx_local_type_template_args;cxx_long_long_type;cxx_noexcept;cxx_nonstatic_member_init;cxx_nullptr;cxx_override;cxx_range_for;cxx_raw_string_literals;cxx_reference_qualified_functions;cxx_right_angle_brackets;cxx_rvalue_references;cxx_sizeof_member;cxx_static_assert;cxx_strong_enums;cxx_thread_local;cxx_trailing_return_types;cxx_unicode_literals;cxx_uniform_initialization;cxx_unrestricted_unions;cxx_user_literals;cxx_variadic_macros;cxx_variadic_templates;cxx_std_14;cxx_aggregate_default_initializers;cxx_attribute_deprecated;cxx_binary_literals;cxx_contextual_conversions;cxx_decltype_auto;cxx_digit_separators;cxx_generic_lambdas;cxx_lambda_init_captures;cxx_relaxed_constexpr;cxx_return_type_deduction;cxx_variable_templates;cxx_std_17;cxx_std_20;cxx_std_23")
set(CMAKE_CXX98_COMPILE_FEATURES "cxx_std_98;cxx_template_template_parameters")
set(CMAKE_CXX11_COMPILE_FEATURES "cxx_std_11;cxx_alias_templates;cxx_alignas;cxx_alignof;cxx_attributes;cxx_auto_type;cxx_constexpr;cxx_decltype;cxx_decltype_incomplete_return_types;cxx_default_function_template_args;cxx_defaulted_functions;cxx_defaulted_move_initializers;cxx_delegating_constructors;cxx_deleted_functions;cxx_enum_forward_declarations;cxx_explicit_conversions;cxx_extended_friend_declarations;cxx_extern_templates;cxx_final;cxx_func_identifier;cxx_generalized_initializers;cxx_inheriting_constructors;cxx_inline_namespaces;cxx_lambdas;cxx_local_type_template_args;cxx_long_long_type;cxx_noexcept;cxx_nonstatic_member_init;cxx_nullptr;cxx_override;cxx_range_for;cxx_raw_string_literals;cxx_reference_qualified_functions;cxx_right_angle_brackets;cxx_rvalue_references;cxx_sizeof_member;cxx_static_assert;cxx_strong_enums;cxx_thread_local;cxx_trailing_return_types;cxx_unicode_literals;cxx_uniform_initialization;cxx_unrestricted_unions;cxx_user_literals;cxx_variadic_macros;cxx_variadic_templates")
set(CMAKE_CXX14_COMPILE_FEATURES "cxx_std_14;cxx_aggregate_default_initializers;cxx_attribute_deprecated;cxx_binary_literals;cxx_contextual_conversions;cxx_decltype_auto;cxx_digit_separators;cxx_generic_lambdas;cxx_lambda_init_captures;cxx_relaxed_constexpr;cxx_return_type_deduction;cxx_variable_templates")
set(CMAKE_CXX17_COMPILE_FEATURES "cxx_std_17")
set(CMAKE_CXX20_COMPILE_FEATURES "cxx_std_20")
set(CMAKE_CXX23_COMPILE_FEATURES "cxx_std_23")
set(CMAKE_CXX26_COMPILE_FEATURES "")

set(CMAKE_CXX_PLATFORM_ID "Linux")
set(CMAKE_CXX_SIMULATE_ID "")
set(CMAKE_CXX_COMPILER_FRONTEND_VARIANT "GNU")
set(CMAKE_CXX_SIMULATE_VERSION "")




set(CMAKE_AR "/nix/store/zx71vq7s1v840wqsrw2m2ckmxn413a2b-gcc-wrapper-13.3.0/bin/ar")
set(CMAKE_CXX_COMPILER_AR "/usr/bin/gcc-ar-13")
set(CMAKE_RANLIB "/nix/store/zx71vq7s1v840wqsrw2m2ckmxn413a2b-gcc-wrapper-13.3.0/bin/ranlib")
set(CMAKE_CXX_COMPILER_RANLIB "/usr/bin/gcc-ranlib-13")
set(CMAKE_LINKER "/nix/store/zx71vq7s1v840wqsrw2m2ckmxn413a2b-gcc-wrapper-13.3.0/bin/ld")
set(CMAKE_LINKER_LINK "")
set(CMAKE_LINKER_LLD "")
set(CMAKE_CXX_COMPILER_LINKER "/nix/store/ds04v6rdcnsvr48aa9dfmkdrb5k3k0vg-binutils-wrapper-2.43.1/bin/ld")
set(CMAKE_CXX_COMPILER_LINKER_ID "GNU")
set(CMAKE_CXX_COMPILER_LINKER_VERSION 2.43.1)
set(CMAKE_CXX_COMPILER_LINKER_FRONTEND_VARIANT GNU)
set(CMAKE_MT "")
set(CMAKE_TAPI "CMAKE_TAPI-NOTFOUND")
set(CMAKE_COMPILER_IS_GNUCXX 1)
set(CMAKE_CXX_COMPILER_LOADED 1)
set(CMAKE_CXX_COMPILER_WORKS TRUE)
set(CMAKE_CXX_ABI_COMPILED TRUE)

set(CMAKE_CXX_COMPILER_ENV_VAR "CXX")

set(CMAKE_CXX_COMPILER_ID_RUN 1)
set(CMAKE_CXX_SOURCE_FILE_EXTENSIONS C;M;c++;cc;cpp;cxx;m;mm;mpp;CPP;ixx;cppm;ccm;cxxm;c++m)
set(CMAKE_CXX_IGNORE_EXTENSIONS inl;h;hpp;HPP;H;o;O;obj;OBJ;def;DEF;rc;RC)

foreach (lang IN ITEMS C OBJC OBJCXX)
  if (CMAKE_${lang}_COMPILER_ID_RUN)
    foreach(extension IN LISTS CMAKE_${lang}_SOURCE_FILE_EXTENSIONS)
      list(REMOVE_ITEM CMAKE_CXX_SOURCE_FILE_EXTENSIONS ${extension})
    endforeach()
  endif()
endforeach()

set(CMAKE_CXX_LINKER_PREFERENCE 30)
set(CMAKE_CXX_LINKER_PREFERENCE_PROPAGATES 1)
set(CMAKE_CXX_LINKER_DEPFILE_SUPPORTED FALSE)

# Save compiler ABI information.
set(CMAKE_CXX_SIZEOF_DATA_PTR "8")
set(CMAKE_CXX_COMPILER_ABI "ELF")
set(CMAKE_CXX_BYTE_ORDER "LITTLE_ENDIAN")
set(CMAKE_CXX_LIBRARY_ARCHITECTURE "")

if(CMAKE_CXX_SIZEOF_DATA_PTR)
  set(CMAKE_SIZEOF_VOID_P "${CMAKE_CXX_SIZEOF_DATA_PTR}")
endif()

if(CMAKE_CXX_COMPILER_ABI)
  set(CMAKE_INTERNAL_PLATFORM_ABI "${CMAKE_CXX_COMPILER_ABI}")
endif()

if(CMAKE_CXX_LIBRARY_ARCHITECTURE)
  set(CMAKE_LIBRARY_ARCHITECTURE "")
endif()

set(CMAKE_CXX_CL_SHOWINCLUDES_PREFIX "")
if(CMAKE_CXX_CL_SHOWINCLUDES_PREFIX)
  set(CMAKE_CL_SHOWINCLUDES_PREFIX "${CMAKE_CXX_CL_SHOWINCLUDES_PREFIX}")
endif()





set(CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES "/nix/store/4zk6nf51a5n5g37x7g3n819p45cpcbnx-raylib-5.0/include;/nix/store/wb1wlhnhwkj8wl9hrrw96a10dwhk003m-glu-9.0.3-dev/include;/nix/store/9qfg08m8sp69471ksam3zq547wv6a94m-libglvnd-1.7.0-dev/include;/nix/store/69i38h2vi9yv0ddll9iynhc7syfspc13-libX11-1.8.10-dev/include;/nix/store/3a0nz1s1kwjkx1yvbr2jbamgzvwfsz6v-xorgproto-2024.1/include;/nix/store/vb0ml7mw7fnylbyi01xwn9fjh3lchbb3-libxcb-1.17.0-dev/include;/nix/store/hh8hrwdav4paz6hvb9rrhjq61yxafk85-mesa-24.2.8-dev/include;/nix/store/3r5ysqp5a99vsba736brxaa8zzmxzj8m-libdrm-2.4.123-dev/include;/nix/store/1kh63vb4z55hwl4s3y52838vxgb42f8s-libXcursor-1.2.2-dev/include;/nix/store/nyjm85lk75ygz0hdfn51f0s1l9yka9rg-libXrandr-1.5.4-dev/include;/nix/store/lw9j96s6417547sw7yv1an8gcyi46vqj-libXrender-0.9.11-dev/include;/nix/store/9v272z68zy7b5bfm95nkdv5g38dl0lj1-libXinerama-1.1.5-dev/include;/nix/store/5afp6wzbhqd2rzwjmbs3hzmvycznal0n-libXi-1.8.2-dev/include;/nix/store/w8fyn7mgb9dq6xh614077sdp5yw9yy4p-libXfixes-6.0.1-dev/include;/nix/store/8zkz0nw2s7hmq1crmxqazj8ivnp940w2-libXext-1.3.6-dev/include;/nix/store/2d34blixaj36bw536pbr6v1w4l63y4cq-libXau-1.0.11-dev/include;/nix/store/xnywx1v41j1gqvp75m7h2xkzbccdj6sy-alsa-lib-1.2.12-dev/include;/nix/store/x0a3pj8nmg4ijssmi8wyyd30v4fhycp7-openjdk-17.0.15+6/include;/nix/store/y0dfznlfxnlwwn9ha31sbbhdhajckhk1-compiler-rt-libc-18.1.8-dev/include;/nix/store/hx59pzhi1x2mfm1yjyg8rf3c9swa4nph-gcc-13.3.0/include/c++/13.3.0;/nix/store/hx59pzhi1x2mfm1yjyg8rf3c9swa4nph-gcc-13.3.0/include/c++/13.3.0/x86_64-unknown-linux-gnu;/nix/store/hx59pzhi1x2mfm1yjyg8rf3c9swa4nph-gcc-13.3.0/include/c++/13.3.0/backward;/nix/store/hx59pzhi1x2mfm1yjyg8rf3c9swa4nph-gcc-13.3.0/lib/gcc/x86_64-unknown-linux-gnu/13.3.0/include;/nix/store/hx59pzhi1x2mfm1yjyg8rf3c9swa4nph-gcc-13.3.0/include;/nix/store/hx59pzhi1x2mfm1yjyg8rf3c9swa4nph-gcc-13.3.0/lib/gcc/x86_64-unknown-linux-gnu/13.3.0/include-fixed;/nix/store/dc9s2cqvwslmx5lsfidnn60v9af044zw-glibc-2.40-66-dev/include")
set(CMAKE_CXX_IMPLICIT_LINK_LIBRARIES "stdc++;m;gcc_s;gcc;c;gcc_s;gcc")
set(CMAKE_CXX_IMPLICIT_LINK_DIRECTORIES "/nix/store/4zk6nf51a5n5g37x7g3n819p45cpcbnx-raylib-5.0/lib;/nix/store/ws1zf8zhnk98yp0b4zkgk5w3s29bxsyl-libglvnd-1.7.0/lib;/nix/store/npfziwvmylm1282hbps24sfdsb8kbr55-glu-9.0.3/lib;/nix/store/09aq563zkqcw9ikxn02p4bm13i2hz51r-libxcb-1.17.0/lib;/nix/store/i88qpfah2j0yq7nihbm626abbhzkc21g-libX11-1.8.10/lib;/nix/store/6qmfmza7nyac6jnk44v6pigqpiadhjh0-libdrm-2.4.123/lib;/nix/store/2lq8ihiy3d2b2xa0m9mnwdqhscczzb2m-mesa-24.2.8/lib;/nix/store/k3z5rl2jqvixzqzi7q1dn77hqcgqjbk3-libXcursor-1.2.2/lib;/nix/store/ps69z8f33spii7xrk3v9w9pwgxnb4cm6-libXrender-0.9.11/lib;/nix/store/gw62nd3f83hvnckgwx761awppy8hbjv1-libXrandr-1.5.4/lib;/nix/store/wqj0fwwng816jqrf7v505f0zy312dixy-libXinerama-1.1.5/lib;/nix/store/qr5knazn342nhn1vqcjfg587bqmxwx86-libXfixes-6.0.1/lib;/nix/store/yajrv8lpsmrypm6ra5sf82whpn1gjqm3-libXau-1.0.11/lib;/nix/store/58hahz0ds522pz7bc9fwmgjjfy36swn8-libXext-1.3.6/lib;/nix/store/rab8q4c4iqr0sqix4z792jipn4k6zs42-libXi-1.8.2/lib;/nix/store/9p4wxazk7pfd0gamidpbmbd8hhhahyn4-alsa-lib-1.2.12/lib;/nix/store/5m9amsvvh2z8sl7jrnc87hzy21glw6k1-glibc-2.40-66/lib;/nix/store/hx59pzhi1x2mfm1yjyg8rf3c9swa4nph-gcc-13.3.0/lib/gcc/x86_64-unknown-linux-gnu/13.3.0;/nix/store/hh698a2nnpqr47lh52n26wi8fiah3hid-gcc-13.3.0-lib/lib;/nix/store/ds04v6rdcnsvr48aa9dfmkdrb5k3k0vg-binutils-wrapper-2.43.1/bin;/nix/store/hx59pzhi1x2mfm1yjyg8rf3c9swa4nph-gcc-13.3.0/lib64;/nix/store/hx59pzhi1x2mfm1yjyg8rf3c9swa4nph-gcc-13.3.0/lib")
set(CMAKE_CXX_IMPLICIT_LINK_FRAMEWORK_DIRECTORIES "")
set(CMAKE_CXX_COMPILER_CLANG_RESOURCE_DIR "")

set(CMAKE_CXX_COMPILER_IMPORT_STD "")
### Imported target for C++23 standard library
set(CMAKE_CXX23_COMPILER_IMPORT_STD_NOT_FOUND_MESSAGE "Toolchain does not support discovering `import std` support")



