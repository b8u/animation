rem *.fxc -> cso (compiled shader object)
fxc         /E main /T ps_5_0 /Fo ps.fxc  ps.hlsl
fxc /Od /Zi /E main /T ps_5_0 /Fo psd.fxc ps.hlsl

fxc         /E main /T vs_5_0 /Fo vs.fxc  vs.hlsl
fxc /Od /Zi /E main /T vs_5_0 /Fo vsd.fxc vs.hlsl
