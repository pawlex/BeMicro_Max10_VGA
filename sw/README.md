# SW and ROM
Ben's orig. design used a 32KiB image, so I just used 'dd' to truncate his binary to 16KiB (size of my on-chip rom).  Then, I converted this to ihex using bin2hex, which is what quartus expects.
