use std::collections::HashMap;

pub enum Sort {
    Alphabetic,
    Numeric,
}

pub type Tally = HashMap<String, i64>; 