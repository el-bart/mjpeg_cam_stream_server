#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>
#include <unistd.h>
#include <string.h>

void check_framerate(int const fd, v4l2_fmtdesc const& fmt, v4l2_frmsizeenum const& frmsize)
{
  struct v4l2_frmivalenum frmsival;
  memset(&frmsival, 0, sizeof(frmsival));
  frmsival.pixel_format = fmt.pixelformat;
  frmsival.width = frmsize.discrete.width;
  frmsival.height = frmsize.discrete.height;
  frmsival.index = 0;

  while (ioctl(fd, VIDIOC_ENUM_FRAMEINTERVALS, &frmsival) == 0)
  {
    if (frmsival.type == V4L2_FRMIVAL_TYPE_DISCRETE)
    {
      printf("    Frame Rate: %.2f fps\n", 1.0 / frmsival.discrete.numerator * frmsival.discrete.denominator);
    }
    else
      if (frmsival.type == V4L2_FRMIVAL_TYPE_STEPWISE)
      {
        float minfps = 1.0 / (frmsival.stepwise.min.numerator / frmsival.stepwise.min.denominator);
        float maxfps = 1.0 / (frmsival.stepwise.max.numerator / frmsival.stepwise.max.denominator);
        float stepfps = 1.0 / (frmsival.stepwise.step.numerator / frmsival.stepwise.step.denominator);
        printf("    Frame Rate Range: %.2f to %.2f fps, step %.2f fps\n", minfps, maxfps, stepfps);
      }
      else
        if (frmsival.type == V4L2_FRMIVAL_TYPE_CONTINUOUS)
        {
          float minfps = 1.0 / (frmsival.stepwise.min.numerator / frmsival.stepwise.min.denominator);
          float maxfps = 1.0 / (frmsival.stepwise.max.numerator / frmsival.stepwise.max.denominator);
          printf("    Frame Rate Range: %.2f to %.2f fps\n", minfps, maxfps);
        }
    frmsival.index++;
  }
}


void check_resolutions(int const fd, v4l2_fmtdesc const& fmt)
{
  struct v4l2_frmsizeenum frmsize;
  frmsize.pixel_format = fmt.pixelformat;
  frmsize.index = 0;

  while (ioctl(fd, VIDIOC_ENUM_FRAMESIZES, &frmsize) == 0)
  {
    if (frmsize.type == V4L2_FRMSIZE_TYPE_DISCRETE)
    {
      printf("  Resolution: %dx%d\n", frmsize.discrete.width, frmsize.discrete.height);
    }
    else
      if (frmsize.type == V4L2_FRMSIZE_TYPE_STEPWISE)
      {
        printf("  Resolution: %dx%d to %dx%d step %dx%d\n",
            frmsize.stepwise.min_width, frmsize.stepwise.min_height,
            frmsize.stepwise.max_width, frmsize.stepwise.max_height,
            frmsize.stepwise.step_width, frmsize.stepwise.step_height);
      }
      else
        if (frmsize.type == V4L2_FRMSIZE_TYPE_CONTINUOUS)
        {
          printf("  Resolution: %dx%d to %dx%d\n",
              frmsize.stepwise.min_width, frmsize.stepwise.min_height,
              frmsize.stepwise.max_width, frmsize.stepwise.max_height);
        }
    check_framerate(fd, fmt, frmsize);
    frmsize.index++;
  }
}


int main()
{
  int fd = open("/dev/video0", O_RDWR);
  if (fd < 0) {
    perror("open");
    return 1;
  }

  // Enumerate pixel formats
#if 0
  struct v4l2_fmtdesc {
    __u32 index;             /* Format number      */
    __u32 type;              /* enum v4l2_buf_type */
    __u32 flags;
    __u8  description[32];   /* Description string */
    __u32 pixelformat;       /* Format fourcc      */
    __u32 mbus_code;         Media bus code    */
    __u32 reserved[3];
  };
#endif
  struct v4l2_fmtdesc fmt;
  fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  fmt.index = 0;
  while (ioctl(fd, VIDIOC_ENUM_FMT, &fmt) == 0)
  {
    printf("Format: %s\n", fmt.description);
    printf("Format (fourcc): 0x%08x\n", fmt.pixelformat);
    check_resolutions(fd, fmt);
    fmt.index++;
  }

  close(fd);
  return 0;
}

