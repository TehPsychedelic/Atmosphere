/*
 * Copyright (c) 2018-2019 Atmosphère-NX
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <stratosphere.hpp>

namespace ams::fs {

    Result FileStorage::UpdateSize() {
        R_UNLESS(this->size == InvalidSize, ResultSuccess());
        return this->base_file->GetSize(&this->size);
    }

    Result FileStorage::Read(s64 offset, void *buffer, size_t size) {
        /* Immediately succeed if there's nothing to read. */
        R_UNLESS(size > 0, ResultSuccess());

        /* Validate buffer. */
        R_UNLESS(buffer != nullptr, fs::ResultNullptrArgument());

        /* Ensure our size is valid. */
        R_TRY(this->UpdateSize());

        /* Ensure our access is valid. */
        R_UNLESS(IStorage::IsRangeValid(offset, size, this->size), fs::ResultOutOfRange());

        size_t read_size;
        return this->base_file->Read(&read_size, offset, buffer, size);
    }

    Result FileStorage::Write(s64 offset, const void *buffer, size_t size) {
        /* Immediately succeed if there's nothing to write. */
        R_UNLESS(size > 0, ResultSuccess());

        /* Validate buffer. */
        R_UNLESS(buffer != nullptr, fs::ResultNullptrArgument());

        /* Ensure our size is valid. */
        R_TRY(this->UpdateSize());

        /* Ensure our access is valid. */
        R_UNLESS(IStorage::IsRangeValid(offset, size, this->size), fs::ResultOutOfRange());

        return this->base_file->Write(offset, buffer, size, fs::WriteOption());
    }

    Result FileStorage::Flush() {
        return this->base_file->Flush();
    }

    Result FileStorage::GetSize(s64 *out_size) {
        R_TRY(this->UpdateSize());
        *out_size = this->size;
        return ResultSuccess();
    }

    Result FileStorage::SetSize(s64 size) {
        this->size = InvalidSize;
        return this->base_file->SetSize(size);
    }

    Result FileStorage::OperateRange(void *dst, size_t dst_size, OperationId op_id, s64 offset, s64 size, const void *src, size_t src_size) {
        switch (op_id) {
            case OperationId::InvalidateCache:
            case OperationId::QueryRange:
                if (size == 0) {
                    if (op_id == OperationId::QueryRange) {
                        R_UNLESS(dst != nullptr,                     fs::ResultNullptrArgument());
                        R_UNLESS(dst_size == sizeof(QueryRangeInfo), fs::ResultInvalidSize());
                        reinterpret_cast<QueryRangeInfo *>(dst)->Clear();
                    }
                    return ResultSuccess();
                }
                R_TRY(this->UpdateSize());
                R_UNLESS(IStorage::IsOffsetAndSizeValid(offset, size), fs::ResultOutOfRange());
                return this->base_file->OperateRange(dst, dst_size, op_id, offset, size, src, src_size);
            default:
                return fs::ResultUnsupportedOperation();
        }
    }

}
