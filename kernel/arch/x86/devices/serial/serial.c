#include "serial.h"

#include <liblox/memory.h>

#include "../../io.h"

static const uint16_t serial_io_ports[4] = {
    0x3F8,
    0x2F8,
    0x3E8,
    0x2E8
};

static uint8_t convert_recv(uint8_t in) {
    switch (in) {
        case 0x7F:
            return 0x08;
        case 0x0D:
            return '\n';
        default:
            return in;
    }
}

static void serial_enable(uint16_t port) {
    outb((uint16_t) (port + 1), 0x00); /* Disable interrupts */
    outb((uint16_t) (port + 3), 0x80); /* Enable divisor mode */
    outb((uint16_t) (port + 0), 0x01); /* Div Low:  01 Set the port to 115200 bps */
    outb((uint16_t) (port + 1), 0x00); /* Div High: 00 */
    outb((uint16_t) (port + 3), 0x03); /* Disable divisor mode, set parity */
    outb((uint16_t) (port + 2), 0xC7); /* Enable FIFO and clear */
    outb((uint16_t) (port + 4), 0x0B); /* Enable interrupts */
    outb((uint16_t) (port + 1), 0x01); /* Enable interrupts */
}

static int serial_rcvd(uint16_t device) {
    return inb((uint16_t) (device + 5)) & 1;
}

static int serial_transmit_empty(uint16_t device) {
    return inb((uint16_t) (device + 5)) & 0x20;
}

static void serial_send(uint16_t device, uint8_t out) {
    while (serial_transmit_empty(device) == 0) {}
    outb(device, out);
}

static void tty_serial_write(tty_t* tty, const uint8_t* buffer, size_t len) {
    tty_serial_t* serial = tty->internal.provider;

    for (size_t i = 0; i < len; i++) {
        uint8_t c = buffer[i];

        serial_send(serial->port, c);
    }
}

static void tty_serial_destroy(tty_t* tty) {
    tty_serial_t* serial = tty->internal.provider;
    ktask_cancel(serial->poll_task);

    free(tty->internal.provider);
    free(tty);
}

static void serial_poll(void* data) {
    tty_t* tty = data;
    tty_serial_t* serial = tty->internal.provider;

    if (serial_rcvd(serial->port) != 0) {
        uint8_t c = inb(serial->port);

        if (!tty->flags.raw) {
            c = convert_recv(c);
        }

        tty_read_event_t event = {
            .data = &c,
            .size = 1,
            .tty = tty
        };

        epipe_deliver(&tty->reads, &event);
    }
}

tty_serial_t* tty_create_serial(char* name, uint index) {
    tty_t* tty = tty_create(name);
    tty_serial_t* serial = zalloc(sizeof(tty_serial_t));
    serial->port = serial_io_ports[index];
    serial->tty = tty;

    serial_enable(serial->port);

    tty->internal.provider = serial;
    tty->ops.write = tty_serial_write;
    tty->ops.destroy = tty_serial_destroy;
    serial->poll_task = ktask_repeat(1, serial_poll, tty);
    return serial;
}
