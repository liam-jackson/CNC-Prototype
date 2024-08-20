#!/usr/bin/env bash
# Finds all .proto files in the protos directory and generates the gRPC Python classes

echo "Generating Python classes from protos directory..."

_dst_dir="../interface-python/classes/pb"

# Check if the destination directory exists, if not, create it
if [ ! -d "${_dst_dir}" ]; then
    echo "${_dst_dir} directory not found, creating..."
    mkdir -p "${_dst_dir}"
fi

# Find all .proto files, excluding those in .history directories, and process them
find "." -name "*.proto" -not -path "*/.history/*" | while read -r proto_file; do
    echo "Processing ${proto_file}..."
    conda run -n cnc \
        python -m grpc_tools.protoc \
        -I. \
        --python_out="${_dst_dir}" \
        --pyi_out="${_dst_dir}" \
        --grpc_python_out="${_dst_dir}" \
        "${proto_file}"
done

echo "Done!"
