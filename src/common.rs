
#[derive(Debug, Clone, PartialEq, Serialize, Deserialize)]
pub enum Request {
    Get(String),
    Set(String, String),
}

#[derive(Debug, Clone, PartialEq, Serialize, Deserialize)]
pub enum Response {
    None,
    Value(String),
}

pub enum Sort {
    Alphabetic,
    Numeric,
}
