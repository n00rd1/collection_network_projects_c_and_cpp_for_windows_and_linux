#ifndef __LIBTYPE_H
#define __LIBTYPE_H
#include <arpa/inet.h>

/**
 * valid_wpa_key: checking and converting a string to a UINT variable
 * @s: attribute value pointer
 * @out: pointer to return result
 * @min: minimal value
 * @max: maximal value
 * returns 0 on success
 */
int valid_type_uint (char *s, unsigned int *out, unsigned int min, unsigned int max);

/**
 * valid_type_bool: checking and converting a string to a boolean variable
 * @s: attribute value pointer
 * @out: pointer to return result
 * returns 0 on success
 */
int valid_type_bool (char *s, unsigned char *out);

/**
 * valid_type_enum: checking the string for belonging to the list of strings
 * @s: attribute value pointer
 * @listenum: attribut value pointer to list
 * returns 0 on success
 */
int valid_type_enum (char *s, const char **listenum);

/**
 * valid_type_string: checking the string for valid characters and length
 * @s: attribute value pointer
 * @min: minimal value
 * @max: maximal value
 * returns 0 on success
 */
int valid_type_string (char *s, unsigned min, unsigned max);

/**
 * valid_type_mac: checking a string to a mac address
 * @s: attribute value pointer
 * returns 0 on success
 */
int valid_type_mac (char *s);

/**
 * valid_type_ip4addr: checking and converting a string to v4 ip addresses
 * @s: attribute value pointer
 * @si_me: pointer to return result on a structure
 * returns 0 on success
 */
int valid_type_ip4addr (char *s, struct sockaddr_in *si_me);

/**
 * valid_type_port: checking and converting a string to a port (a special case of valid_type_int )
 * @s: attribute value pointer
 * @out: pointer to return result
 * returns 0 on success
 */
int valid_type_port (char *s, unsigned int *out);

/**
 * valid_type_hostname: checking for compliance with the hostname type
 * @s: attribute value pointer
 * returns 0 on success
 */
int valid_type_hostname (char *s);

/**
 * valid_type_host: checking for compliance with the host and ipv4 type
 * @s: attribute value pointer
 * returns 0 on success
 */
int valid_type_host (char *s);

/**
 * valid_type_time: checking and converting for compliance with the time type (1m, 10m, 5h), min and max are expressed in seconds
 * @s: attribute value pointer
 * @out: pointer to return result
 * @min: minimal value
 * @max: maximal value
 * returns 0 on success
 */
int valid_type_time (char *s, unsigned int *out, unsigned int min, unsigned int max);

/**
 * valid_wpa_key: checking for compliance wpa key 
 * @s: attribute value pointer
 * returns 0 on success
 */
int valid_wpa_key (char *s);

#endif
