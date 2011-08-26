# use pkg-config for getting CFLAGS abd LDFLAGS
#FFMPEG_LIBS=libavdevice libavformat libavfilter libavcodec libswscale libavutil
CFLAGS+= -m32 -I../../
#$(shell pkg-config  --cflags $(FFMPEG_LIBS))
LDFLAGS+= -m32 -L../../libavcodec -L../../libavformat -L../../libavutil -L../../libavfilter -L../../libswscale -lswscale -lavcodec -lavformat -lavfilter -lavutil -lbz2 -lz
#$(shell pkg-config --libs $(FFMPEG_LIBS))

EXAMPLES=encoding filtering metadata muxing

OBJS=$(addsuffix .o,$(EXAMPLES))

%: %.o
	$(CC) $< $(LDFLAGS) -o $@

%.o: %.c
	$(CC) $< $(CFLAGS) -c -o $@

.phony: all clean

all: $(OBJS) $(EXAMPLES)

clean:
	rm -rf $(EXAMPLES) $(OBJS)
