meta:
  id: dmag
  file-extension: dmag
  endian: le
  bit-endian: le
  
seq:
  - id: magic
    contents: "DMAG"
  - id: size_x
    type: u1
  - id: size_y
    type: u1
  - id: file_name
    size: 20
    type: str
    encoding: utf-8
  - id: opponent
    type: u1
  - id: data
    type: frame(size_x * size_y * 32 * 4)
    repeat: eos
    
types:
  frame:
    params:
      - id: size
        type: u4
    seq:
      - id: frame_num
        type: u4
      - id: chunk
        size: size
