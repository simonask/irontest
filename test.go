package main

import (
    "fmt"
    "net/http"
)

func main() {
    http.HandleFunc("/", func (w http.ResponseWriter, r *http.Request) {
        fmt.Fprintf(w, "Hello, World!")
    })
    fmt.Printf("Go server listening on :3001")
    http.ListenAndServe(":3001", nil)
}

