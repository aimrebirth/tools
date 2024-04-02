meta:
  id: aim_db_tab
  endian: le
  
seq:
  - id: header
    type: tab
  - id: tables
    type: table
    repeat: expr
    repeat-expr: header.n_tables
  - id: fields
    type: field
    repeat: expr
    repeat-expr: header.n_fields

types:
  char20:
    seq:
        - id: str
          type: str
          encoding: ASCII
          size: 0x20

  tab:
    seq:
        - id: n_tables
          type: u4
        - id: n_fields
          type: u4
          
  table:
    seq:
        - id: id
          type: u4
        - id: name
          type: char20
        - id: unk
          type: u4
          
  field:
    seq:
        - id: table_id
          type: u4
        - id: id
          type: u4
        - id: name
          type: char20
        - id: field_type
          type: u4
          