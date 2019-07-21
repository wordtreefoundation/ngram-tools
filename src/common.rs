
#[derive(Debug, Clone, PartialEq, Serialize, Deserialize)]
pub enum Request {
    Get(String),
}

#[derive(Debug, Clone, PartialEq, Serialize, Deserialize)]
pub enum Response {
    None,
    Value(u32),
}

pub enum Sort {
    Alphabetic,
    Numeric,
}
