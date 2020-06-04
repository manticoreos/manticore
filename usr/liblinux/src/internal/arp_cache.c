/*
 * ARP cache.
 */

#include "internal/arp_cache.h"

#include <string.h>

/* Initialize ARP cache.  */
void arp_cache_init(struct arp_cache *cache)
{
	memset(cache, 0, sizeof(*cache));
}

/* Hash an IP address.  */
static inline uint32_t arp_hash(in_addr_t ip_addr)
{
	/* Multiplicative hashing.  */
	return (ip_addr * (uint32_t)2654435761U) >> (32 - ARP_CACHE_SHIFT);
}

/* Find an entry for IP address.

   This function searches the ARP cache for an entry for IP address using
   linear probe. The probe starts from the index of the provided hash modulo
   cache size, and proceeds to scan the whole cache until a match for provided
   IP address is found.

   See find_occupied_entry() and find_empty_entry() for users of this
   function.

   If succesful, returns a pointer to the entry; otherwise returns NULL.  */
static struct arp_cache_entry *find_entry(struct arp_cache *cache, in_addr_t ip_addr, uint32_t hash)
{
	for (uint32_t count = 0; count < ARP_CACHE_SIZE; count++) {
		size_t idx = (hash + count) & (ARP_CACHE_SIZE - 1);
		struct arp_cache_entry *entry = &cache->entries[idx];

		if (__builtin_expect(entry->ip_addr == ip_addr, 1)) {
			return entry;
		}
	}
	return NULL;
}

/* Find an occupied entry for IP address.

   If succesful, returns a pointer to the entry; otherwise returns NULL.  */
static struct arp_cache_entry *find_occupied_entry(struct arp_cache *cache, in_addr_t ip_addr)
{
	return find_entry(cache, ip_addr, arp_hash(ip_addr));
}

/* Find an empty entry for IP address.

   If succesful, returns a pointer to the entry; otherwise returns NULL.  */
static struct arp_cache_entry *find_empty_entry(struct arp_cache *cache, in_addr_t ip_addr)
{
	/* IP address 0.0.0.0 represents any IP address and, therefore,
	   can have no MAC address.  Let's use it to represent an empty
	   slot.  */
	return find_entry(cache, 0, arp_hash(ip_addr));
}

/* Insert IP and MAC address tuple to the ARP cache.  */
void arp_cache_insert(struct arp_cache *cache, in_addr_t ip_addr, uint8_t *mac_addr)
{
	struct arp_cache_entry *entry = find_occupied_entry(cache, ip_addr);
	if (!entry) {
		entry = find_empty_entry(cache, ip_addr);
	}
	entry->ip_addr = ip_addr;
	memcpy(entry->mac_addr, mac_addr, ETH_ALEN);
}

/* Find the MAC address for an IP address.

   If successful, returns a pointer to the MAC address; otherise returns NULL.  */
uint8_t *arp_cache_lookup(struct arp_cache *cache, in_addr_t ip_addr)
{
	struct arp_cache_entry *entry = find_occupied_entry(cache, ip_addr);
	if (entry && entry->ip_addr == ip_addr) {
		return entry->mac_addr;
	}
	return NULL;
}
