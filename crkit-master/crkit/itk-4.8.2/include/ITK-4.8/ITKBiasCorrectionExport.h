
#ifndef ITKBiasCorrection_EXPORT_H
#define ITKBiasCorrection_EXPORT_H

#ifdef ITK_STATIC
#  define ITKBiasCorrection_EXPORT
#  define ITKBiasCorrection_HIDDEN
#else
#  ifndef ITKBiasCorrection_EXPORT
#    ifdef ITKBiasCorrection_EXPORTS
        /* We are building this library */
#      define ITKBiasCorrection_EXPORT __attribute__((visibility("default")))
#    else
        /* We are using this library */
#      define ITKBiasCorrection_EXPORT __attribute__((visibility("default")))
#    endif
#  endif

#  ifndef ITKBiasCorrection_HIDDEN
#    define ITKBiasCorrection_HIDDEN __attribute__((visibility("hidden")))
#  endif
#endif

#ifndef ITKBIASCORRECTION_DEPRECATED
#  define ITKBIASCORRECTION_DEPRECATED __attribute__ ((__deprecated__))
#endif

#ifndef ITKBIASCORRECTION_DEPRECATED_EXPORT
#  define ITKBIASCORRECTION_DEPRECATED_EXPORT ITKBiasCorrection_EXPORT ITKBIASCORRECTION_DEPRECATED
#endif

#ifndef ITKBIASCORRECTION_DEPRECATED_NO_EXPORT
#  define ITKBIASCORRECTION_DEPRECATED_NO_EXPORT ITKBiasCorrection_HIDDEN ITKBIASCORRECTION_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef ITKBIASCORRECTION_NO_DEPRECATED
#    define ITKBIASCORRECTION_NO_DEPRECATED
#  endif
#endif

#endif
