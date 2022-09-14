
struct arp_header
{
    uint8_t hw_type;
    uint8_t proto_type;

    ipv4_addr_t xx_target;
}
