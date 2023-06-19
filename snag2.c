#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <linux/fb.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define FRAMEBUFFER_0 "/dev/fb0"
#define FRAMEBUFFER_1 "/dev/fb1"

typedef struct {
    unsigned char red;
    unsigned char green;
    unsigned char blue;
} ColorRGB;

typedef unsigned char Color1bit;

int main() {
    //int cur_page = 1; //TO DO - double buffering
    int fb0, fb1;
    struct fb_var_screeninfo var_info_0, var_info_1;
    size_t screen_size_0, screen_size_1;
    char *fb0_data, *fb1_data;
    unsigned int x, y;
    double x_ratio, y_ratio;
    unsigned int x_scaled, y_scaled;
    uint *fb0_pixel_rgb;
    Color1bit *fb1_pixel;
    bool hasChanged;

    // Open framebuffer /dev/fb0 for reading
    fb0 = open(FRAMEBUFFER_0, O_RDONLY);
    if (fb0 == -1) {
        perror("Error opening framebuffer /dev/fb0");
        return 1;
    }

    // Open framebuffer /dev/fb1 for reading/writing
    fb1 = open(FRAMEBUFFER_1, O_RDWR);
    if (fb1 == -1) {
        perror("Error opening framebuffer /dev/fb1");
        close(fb0);
        return 1;
    }

    // Get variable screen information for /dev/fb0
    if (ioctl(fb0, FBIOGET_VSCREENINFO, &var_info_0) == -1) {
        perror("Error retrieving variable screen information for /dev/fb0");
        close(fb0);
        close(fb1);
        return 1;
    }

    // Get variable screen information for /dev/fb1
    if (ioctl(fb1, FBIOGET_VSCREENINFO, &var_info_1) == -1) {
        perror("Error retrieving variable screen information for /dev/fb1");
        close(fb0);
        close(fb1);
        return 1;
    }

    // Calculate screen size in bytes for /dev/fb0
    screen_size_0 = var_info_0.yres_virtual * var_info_0.xres_virtual * var_info_0.bits_per_pixel / 8;

    // Calculate screen size in bytes for /dev/fb1
    screen_size_1 = var_info_1.yres_virtual * var_info_1.xres_virtual * sizeof(Color1bit);// * 2; // Double height for back buffer

    // Map the framebuffer memory for /dev/fb0
    fb0_data = mmap(0, screen_size_0, PROT_READ, MAP_PRIVATE, fb0, 0);
    if (fb0_data == MAP_FAILED) {
        perror("Error mapping framebuffer /dev/fb0");
        close(fb0);
        close(fb1);
        return 1;
    }

    // Map the framebuffer memory for /dev/fb1
    fb1_data = mmap(0, screen_size_1, PROT_READ | PROT_WRITE, MAP_SHARED, fb1, 0);
    if (fb1_data == MAP_FAILED) {
        perror("Error mapping framebuffer /dev/fb1");
        munmap(fb0_data, screen_size_0);
        close(fb0);
        close(fb1);
        return 1;
    }

    // Calculate scaling ratios
    x_ratio = (double) var_info_0.xres_virtual / 400;
    y_ratio = (double) var_info_0.yres_virtual / 240;

    bool running = true;
    uint bit_pixel; // 1 or 0
    
    while (running) {
        // Perform pixel copy and scaling
        for (y = 0; y < 240; ++y) {
            for (x = 0; x < 400; ++x) {
                // Calculate scaled coordinates
                x_scaled = (unsigned int) (x * x_ratio);
                y_scaled = (unsigned int) (y * y_ratio);

                // Get pixel from /dev/fb0
                fb0_pixel_rgb = (ColorRGB *) (fb0_data + (y_scaled * var_info_0.xres_virtual + x_scaled) * sizeof(ColorRGB));
                
                // Convert RGB to grayscale
                unsigned char gray = (unsigned char) (0.299 * fb0_pixel_rgb->red +
                                                      0.587 * fb0_pixel_rgb->green +
                                                      0.114 * fb0_pixel_rgb->blue);
                
                // Convert grayscale to binary
                bit_pixel = gray > 127 ? 1 : 0; // should I make this 0b10000000 / 0b00000000
                
                // Get pixel from /dev/fb1
                //fb1_pixel_gray = (ColorGray *) (fb1_data + (y * var_info_1.xres_virtual + x) * sizeof(ColorGray)); // * cur_page
                fb1_pixel = (Color1bit *) (fb1_data + (y * var_info_1.xres_virtual + x) * sizeof(Color1bit)); // * cur_page
                

                // Check if the pixel color has changed
                //hasChanged = (*fb1_pixel_gray != gray);
                //if (hasChanged) {
                    *fb1_pixel = bit_pixel;
                //}
            }
        }

        // Update back-buffer page
        //cur_page = cur_page = 0 ? 1 : 0; //TO DO - double buffering

        // Flip pages - FAILS!
        //var_info_1.yoffset = cur_page * var_info_1.yres;
        //var_info_1.activate = FB_ACTIVATE_VBL;
        //if (ioctl(fb1, FBIOPAN_DISPLAY, &var_info_1) == -1) {
        //    printf("Error panning display.\n");
        //}
        //else{
        //    printf("Panned display!");
        //}

        // Update panning offset for /dev/fb1 to switch between double-height backbuffers
        
        // FAILED WAITFORVSYNC not implemented?
        // if (ioctl(fb1, FBIO_WAITFORVSYNC, 0) == 0) {
        //     var_info_1.yoffset = cur_page * var_info_1.yres;
        //     if (ioctl(fb1, FBIOPAN_DISPLAY, &var_info_1)) {
        //         printf("Pan failed.\n");
        //     }
        // }
        // else {
        //     printf("VSync failed.\n");
        // }

        // FAILED
        //if (ioctl(fb1, FBIOPAN_DISPLAY, &var_info_1) == -1) {
        //    perror("Error updating panning offset for /dev/fb1");
        //    break;
        //}

        // Test - Do we want to exit the loop?
        char input;
        printf("Press 'q' to quit or any other key to continue: ");
        scanf(" %c", &input);
        if (input == 'q')
            running = false;
    }

    // Cleanup and close file descriptors
    munmap(fb1_data, screen_size_1);
    munmap(fb0_data, screen_size_0);
    close(fb0);
    close(fb1);

    printf("Framebuffer copy complete.\n");

    return 0;
}
