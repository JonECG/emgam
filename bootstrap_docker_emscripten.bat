docker run^
 --rm^
 -v %cd%:/src^
 emscripten/emsdk^
 /bin/bash -c "./bootstrap_emscripten.sh && ./bootstrap_emscripten_make.sh"
pause