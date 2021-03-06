#ifndef __PAINTLIB_VERSION_H__
#define __PAINTLIB_VERSION_H__

#undef __BEGIN_DECLS
#undef __END_DECLS
#ifdef __cplusplus
# define __BEGIN_DECLS extern "C" {
# define __END_DECLS }
#else
# define __BEGIN_DECLS /* empty */
# define __END_DECLS /* empty */
#endif
__BEGIN_DECLS


#define PAINTLIB_VERSION "2.6.1"

extern const char * paintlib_version;

__END_DECLS

#endif /* __GSL_VERSION_H__ */
