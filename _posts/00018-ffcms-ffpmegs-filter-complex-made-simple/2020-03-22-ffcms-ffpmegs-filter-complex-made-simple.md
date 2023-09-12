---
layout: post_without_latex
title:  "ffcms - FFmpeg's -filter_complex made simple"
date:   2020-03-22 00:00:00 +0200
categories: cpp
published: true
---


# ffcms - FFmpeg's -filter_complex made simple

This article is not meant to dive into a complete solution or tool. It's going to describe a concept and a simple script that I'm working on and I think will make my relationship with [FFmpeg](https://ffmpeg.org/) easier. Maybe yours too? (:

(Yes I know that the FFmpeg project provides more than only the ffmpeg binary. But, in this art, when saying FFmpeg I mean the ffmpeg binary only.)


# FFmpeg

FFmpeg is a very useful tool. It comes with a variety of options, codecs and generally stuff that you may need while operating on media files.

For simple tasks, the tool is very straightforward. You want to convert a video? Beng: ffmpeg -i input.mov output.mp4. You want to remove audio from a video? Beng beng: ffmpeg -i input.mov -an output.mov.

Simplicity is very important but there are cases when you need to do more complex stuff. E.g. you want to put a video on top of another, bigger one, then you want to adjust colors a little and then...

Yee.. The wishlist can escalate quickly. But, fear no more! FFmpeg is right here for you. Well, you just need to know how to use it. So you google it. FFmpeg put video on another. You quickly find out that it is called overlaying and you must use a filter named, you guessed it, overlay. Fortunately FFmpeg's community is helpful and you find similar problem at stackoverflow. The command is:

```
ffmpeg -i input.mov -i overlay.mov \
-filter_complex "[1:v]setpts=PTS-10/TB[a]; \
                 [0:v][a]overlay=enable=gte(t\,5):shortest=1[out]" \
-map [out] -map 0:a \
-c:v libx264 -crf 18 -pix_fmt yuv420p \
-c:a copy \
output.mov
```

Back in the days, when I found an answer like this I was like

![foo](https://i.imgur.com/wyfFxnq.jpg)

At the end, it's not THAT bad. You can actually guess what some of these mean:

- input.mov and overlay.mov are probably videos that I want to use.
- -i is used twice, which is strange, but it's always followed by a video that I have on disk, so it probably means input.
- -filter_complex it's followed by a string which.. ee.. has square brackets.. oh, there is a Python slicing! It's a Python script? It computes 10 / terabyte, substracts it from PTS and assigns it to a variable called setpts. Hmm, why is there a semicolon? Python doesn't need them. I'll get back to it later..
- -map so we map [out] whathever that is and then we -map another Thing. Why not? I'll take that.
- -c:v.. I refuse.

Of course, that's an exaggeration. But, there is a grain of truth in it, don't you think?


# ffcms

The -filter_complex argument is the one that the article is about. I want to describe an idea for a tool that simplifies constructing FFmpeg command that is supposed to use -filter_complex.

(Yep, I know that more arguments understand filtering graph syntax, but let's not bother about it here)

# The idea

The concept is actually very simple. Instead of writing a FFmpeg command at once in terminal, you write a JSON that describes what the command's supposed to do. Then you pass the JSON to ffcms tool and it prints a complete ffmpeg command that just can't wait to be run. If you're a visual person:

![foo](/assets/00018-ffcms-ffpmegs-filter-complex-made-simple/wunsz.png)

# Examples

## Original 2x2 grid

FFmpeg provides an example of -filter_complex usage to create a grid out of one video. It can be found here: [https://trac.ffmpeg.org/wiki/FilteringGuide#Multipleinputoverlayin2x2grid](https://trac.ffmpeg.org/wiki/FilteringGuide#Multipleinputoverlayin2x2grid).

The command there is:

```
ffmpeg -f lavfi -i testsrc -f lavfi -i testsrc -f lavfi -i testsrc -f lavfi -i testsrc -filter_complex \
"[1:v]negate[a]; \
 [2:v]hflip[b]; \
 [3:v]edgedetect[c]; \
 [0:v][a]hstack=inputs=2[top]; \
 [b][c]hstack=inputs=2[bottom]; \
 [top][bottom]vstack=inputs=2[out]" -map "[out]" -c:v ffv1 -t 5 multiple_input_grid.avi
```

Let's break it down:

- ffmpeg - The FFmpeg binary you want to run.
- -f lavfi -i testsrc - declaration of an input. The input format is lavfi, the input file is testsrc. This is duplicated four times to imitate four different inputs.
- -filter_complex - that's the place where magic takes place.
- [1:v]negate[a] - Take the video from input at index 1, use negate filter on it and 'save' output of the filter to a link named a.
- [2:v]hflip[b] - Take the video from input at index 2, use hflip filter on it and 'save' output of the filter to a link named b.
- [3:v]edgedetect[c] - Take the video from input at index 3, use edgedetect filter on it and 'save' output of the filter to a link named c.
- [0:v][a]hstack=inputs=2[top] - Take two inputs: the video from input at index 0 and something that is in the link a (which is output of [1:v]negate filtering). Then, on these two inputs, perform filter named hstack with one parameter inputs that is set to 2. Save the result in link named top
- [b][c]hstack=inputs=2[bottom] - Similar to above. Take two inputs from links b and c perform filter hstack with parameter inputs set to 2. Save the result in link named bottom.
- [top][bottom]vstack=inputs=2[out] - take two inputs from links top and bottom, perform vstack filter with param inputs set to 2 and save the result in link named out.
- -map - map stuff from link named out as input to next arguments.
- -c:v ffv1 - use [FFV1 codec](https://en.wikipedia.org/wiki/FFV1) for video.
- -t 5 - result should be a 5 second video
- multiple_input_grid.avi - the name of the output video.
Whoa, that's a lot. The result is something like this (source: [https://trac.ffmpeg.org/attachment/wiki/FilteringGuide/multipleinputoverlay.jpg](https://trac.ffmpeg.org/attachment/wiki/FilteringGuide/multipleinputoverlay.jpg)):

![foo](/assets/00018-ffcms-ffpmegs-filter-complex-made-simple/multiple_input_overlay.jpg)


## ffcms style

I don't have the original `testsrc` file, so I'll use part of the timelapse video that I made for my fiancée. Here's a gif made out of the original video:

![foo](/assets/00018-ffcms-ffpmegs-filter-complex-made-simple/timelapse_compressed.gif)

(BTW, she paints the best mugs in the world, check it out: [ig/fingers.in.paint](https://www.instagram.com/fingers.in.paint/))

To be 99% accurate, let's change the above ffmpeg command to use my timelapse.mov (the 1% is the removed -f lavfi arguments):
```
ffmpeg -i timelapse.mov -i timelapse.mov -i timelapse.mov -i timelapse.mov -filter_complex \
"[1:v]negate[a]; \
 [2:v]hflip[b]; \
 [3:v]edgedetect[c]; \
 [0:v][a]hstack=inputs=2[top]; \
 [b][c]hstack=inputs=2[bottom]; \
 [top][bottom]vstack=inputs=2[out]" -map "[out]" -c:v ffv1 -t 5 timelapse_grid.avi
```

Now let's write a JSON file that ffcms would convert to a command like the above one. Here's the whole JSON:
```json
{
  "in": [
    {
      "id": "first_input",
      "file": "timelapse.mov"
    },
    {
      "id": "second_input",
      "file": "timelapse.mov"
    },
    {
      "id": "third_input",
      "file": "timelapse.mov"
    },
    {
      "id": "fourth_input",
      "file": "timelapse.mov"
    }
  ],

  "out": "timelapse_grid.avi",

  "filters": [
    {
      "in": "second_input",
      "filter": "negate",
      "out": "a"
    },
    {
      "in": "third_input",
      "filter": "hflip",
      "out": "b"
    },
    {
      "in": "fourth_input",
      "filter": "edgedetect",
      "out": "c"
    },
    {
      "in": [
        "first_input",
        "a"
      ],
      "filter": "hstack",
      "out": "top"
    },
    {
      "in": [
        "b",
        "c"
      ],
      "filter": "hstack",
      "out": "bottom"
    },
    {
      "in": [
        "top",
        "bottom"
      ],
      "filter": "vstack",
      "out": "out"
    }
  ]
}
```

Breaking down.

### "in"

The top-level `"in"` is an array of inputs that you'd pass after -i argument to ffmpeg binary. They are in the same order as you'd write them in the command line. It's used to let ffcms know what the input media files are and to assign a meaningful "id"s to the files. Thanks to that, in `"filters"` you can refer to them using e.g. `first_input`, not `0:v`.

### "out"

The top-level `"out"` is just the name of the output video.

### "filters"

The top-level `"filters"` are the filters that you'd write in the string provided to `-filter_complex` argument.

E.g. the first filter:
```json
{
    "in": "second_input",
    "filter": "negate",
    "out": "a"
}
```

is equivalent of the `[1:v]negate[a]` filter in `-filter_complex`'s string.

Mind the `hstack` and `vstack` filters. In `-filter_complex` string you need to pass number of inputs like: `[b][c]hstack=inputs=2[bottom]`. In ffcms JSON you write:
```json
{
    "in": [
        "b",
        "c"
    ],
    "filter": "hstack",
    "out": "bottom"
}
```

The number of inputs is well-known. It's the number of elements in the "in" array, so ffcms adds inputs=2 to the result command on its own.

### Renaming ids
Actually, I don't like the names of the ids. a, b and c? Let's make them more meaningful:

```json
"filters": [
    {
        "in": "second_input",
        "filter": "negate",
        "out": "negated"
    },
    {
        "in": "third_input",
        "filter": "hflip",
        "out": "flipped"
    },
    {
        "in": "fourth_input",
        "filter": "edgedetect",
        "out": "edged"
    },
    {
        "in": [
            "first_input",
            "negated"
        ],
        "filter": "hstack",
        "out": "top"
    },
    {
        "in": [
            "flipped",
            "edged"
        ],
        "filter": "hstack",
        "out": "bottom"
    },
    {
        "in": [
            "top",
            "bottom"
        ],
        "filter": "vstack",
        "out": "out"
    }
]
```

### FFmpeg command
After creating the JSON, you can pass it to ffcms. With the JSON above it prints such FFmpeg command:

```bash
ffmpeg -y -i timelapse.mov -i timelapse.mov -i timelapse.mov -i timelapse.mov -filter_complex \
"[1:v]negate[negated]; \
 [2:v]hflip[flipped]; \
 [3:v]edgedetect[edged]; \
 [0:v][negated]hstack=inputs=2[top]; \
 [flipped][edged]hstack=inputs=2[bottom]; \
 [top][bottom]vstack=inputs=2[out]" \
 -map "[out]" -c:v ffv1 timelapse_grid.avi
```

Now you can just grab it and run in terminal.

Here you can see a gif created out of the result video:

![foo](/assets/00018-ffcms-ffpmegs-filter-complex-made-simple/timelapse_grid_compressed.gif)


## Cameraman
Now let's go further and make our wishlist a little more complex. Let's create a grid, like in the previous example. Additionally, let's overlay an image on top of edged video part. We want to overlay it at given x and y. Plus, we want to make it visible for the first three seconds only.

First of, the most important. The Cameraman:

![foo](/assets/00018-ffcms-ffpmegs-filter-complex-made-simple/cameraman.png)


Now let's adjust JSON to handle additional requirements.

### Adding input media
We need to let ffcms know that there is another input media file. Let's add it to the top-level "in" array:

```json
"in": [
    {
        "id": "cameraman",
        "file": "cameraman.png"
    }
]
```
We need to add one more filter to the top-level filters list - overlay. There we use edged and cameraman as its inputs:

```json
"filters": [
    {
        "in": [
            "edged",
            "cameraman"
        ]
    }
]
```
Next we define the filter:

```json
"filters": [
    {
        "in": [
            "edged",
            "cameraman"
        ],
        "filter": {
            "name": "overlay",
            "params": {
                "x": 200,
                "y": 100,
                "enable": "'between(t,0,3)'"
            }
        }
    }
]
```
x and y are obvious but enable not so much. The enable parameter tells when the cameraman input should be visible on top of the edged one. We specify here, that it should be visible when time (t) is between 0 and 3 second.

Now, the last thing is to specify the output link id:

```json
"filters": [
    {
        "in": [
            "edged",
            "cameraman"
        ],
        "filter": {
            "name": "overlay",
            "params": {
                "x": 200,
                "y": 100,
                "enable": "'between(t,0,3)'"
            }
        },
        "out": "cameraman_on_edges"
    }
]
```
Cool. Now we just need to use cameraman_on_edges in one of the hstack filters:

```json
"filters": [
    {
        "in": [
            "flipped",
            "cameraman_on_edges"
        ],
        "filter": "hstack",
        "out": "bottom"
    }
]
```

And that's it. The full JSON looks like this:
```json
{
    "in": [
        {
            "id": "first_input",
            "file": "timelapse.mov"
        },
        {
            "id": "second_input",
            "file": "timelapse.mov"
        },
        {
            "id": "third_input",
            "file": "timelapse.mov"
        },
        {
            "id": "fourth_input",
            "file": "timelapse.mov"
        },
        {
            "id": "cameraman",
            "file": "cameraman.png"
        }
    ],

    "out": "timelapse_grid.avi",

    "filters": [
        {
            "in": [
                "second_input"
            ],
            "filter": "negate",
            "out": "negated"
        },
        {
            "in": [
                "third_input"
            ],
            "filter": "hflip",
            "out": "flipped"
        },
        {
            "in": [
                "fourth_input"
            ],
            "filter": "edgedetect",
            "out": "edged"
        },
        {
            "in": [
                "edged",
                "cameraman"
            ],
            "filter": {
                "name": "overlay",
                "params": {
                    "x": 200,
                    "y": 100,
                    "enable": "'between(t,0,3)'"
                }
            },
            "out": "cameraman_on_edges"
        },
        {
            "in": [
                "first_input",
                "negated"
            ],
            "filter": "hstack",
            "out": "top"
        },
        {
            "in": [
                "flipped",
                "cameraman_on_edges"
            ],
            "filter": "hstack",
            "out": "bottom"
        },
        {
            "in": [
                "top",
                "bottom"
            ],
            "filter": "vstack",
            "out": "out"
        }
    ]
}
```
ffcms outputs such command:

```bash
ffmpeg -i timelapse.mov -i timelapse.mov -i timelapse.mov -i timelapse.mov -i cameraman.png -filter_complex \
"[1:v]negate[negated]; \
 [2:v]hflip[flipped]; \
 [3:v]edgedetect[edged]; \
 [edged][4:v]overlay=enable='between(t,0,3)':x=200:y=100[cameraman_on_edges]; \
 [0:v][negated]hstack=inputs=2[top]; \
 [flipped][cameraman_on_edges]hstack=inputs=2[bottom]; \
 [top][bottom]vstack=inputs=2[out]" \
 -map "[out]" -c:v ffv1 timelapse_grid.avi
```

and the result video looks like this:

![foo](/assets/00018-ffcms-ffpmegs-filter-complex-made-simple/timelapse_grid_cameraman_compressed.gif)




# Conclusion
Please note that ffcms is not a finished tool. There is still much work to do. Contributions are welcome (: Nevertheless, it's usable enough for me to use it with helping my fiancée with her stuff, so that's for sure a good start.


Discussion and links
- [The FFmpeg project](https://www.ffmpeg.org/)
- [FilteringGuide](https://trac.ffmpeg.org/wiki/FilteringGuide)
- [GitHub repo](https://github.com/stryku/ffcms)

If you have any thoughts, let me know using one of these:

- [GitHub issue](https://github.com/stryku/ffcms/issues)
- [r/ffmpeg](https://www.reddit.com/r/ffmpeg/comments/fn4s27/ffcms_ffmpegs_filter_complex_made_simple/?utm_source=share&utm_medium=web2x)
- [Hacker News](https://news.ycombinator.com/item?id=22657447)


# Thanks for reading o/
 