/**
 * This file is part of radio_tool.
 * Copyright (c) 2020 Kieran Harkin <kieran+git@harkin.me>
 * 
 * radio_tool is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * radio_tool is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with radio_tool. If not, see <https://www.gnu.org/licenses/>.
 */
#include <radio_tool/fw/tyt_fw.hpp>

using namespace radio_tool::fw;

auto TYTFW::Read(const std::string &file) -> void
{
    const auto HeaderSize = 0x100;

    auto i = std::ifstream(file, std::ios_base::binary);
    if (i.is_open())
    {
        auto header = ReadHeader(i);
        CheckHeader(header);

        radio = std::string(header.radio, header.radio + strlen((const char *)header.radio));
        counterMagic = std::vector<uint8_t>(header.counter_magic, header.counter_magic + 1 + header.counter_magic[0]);

        auto binarySize = 0;
        for (auto nMem = 0; nMem < header.n_regions; nMem++)
        {
            uint32_t rStart = 0, rLength = 0;
            i.read((char *)&rStart, 4);
            i.read((char *)&rLength, 4);
            memory_ranges.push_back(std::make_pair(rStart, rLength));
            binarySize += rLength;
        }

        //skip to binary
        i.seekg(HeaderSize);

        data.resize(binarySize);
        i.read((char *)data.data(), data.size());

        //meh ignore footer
    }
    i.close();
}

auto TYTFW::Write(const std::string &file) -> void
{
}

auto TYTFW::ToString() const -> std::string
{
    std::stringstream out;
    auto radioMagic = GetRadioFromMagic(counterMagic);
    out << "Radio: " << radio << " (" << radioMagic << ")" << std::endl
        << "Size:  " << std::fixed << std::setprecision(2) << (data.size() / 1000.0) << " KB" << std::endl
        << "Memory Map: " << std::endl;
    auto n = 0;
    for (const auto &m : memory_ranges)
    {
        out << "  " << n++ << ": Start=0x" << std::setfill('0') << std::setw(8) << std::hex << m.first
            << ", Length=0x" << std::setfill('0') << std::setw(8) << std::hex << m.second
            << std::endl;
    }
    return out.str();
}

auto TYTFW::ReadHeader(std::ifstream &i) -> TYTFirmwareHeader
{
    TYTFirmwareHeader ret = {};
    i.seekg(0, i.beg);
    i.read((char *)&ret, sizeof(TYTFirmwareHeader));

    if (ret.n_regions == std::numeric_limits<uint32_t>::max())
    {
        ret.n_regions = 1; //if 0xFFFFFFFF then assume 1
    }

    return ret;
}

auto TYTFW::CheckHeader(const TYTFirmwareHeader &header) -> void
{
    if (!std::equal(tyt::magic::begin.begin(), tyt::magic::begin.end(), header.magic))
    {
        throw std::runtime_error("Invalid start magic");
    }

    if (header.counter_magic[0] > 3)
    {
        throw std::runtime_error("Invalid counter magic length");
    }

    auto magic_match = false;
    for (const auto &r : tyt::magic::All)
    {
        //header.counter_magic should always be longer than r.second
        if (std::equal(r.second.begin(), r.second.end(), header.counter_magic))
        {
            magic_match = true;
            break;
        }
    }
    if (!magic_match)
    {
        throw std::runtime_error("Counter magic is invalid, or not supported");
    }

    uint32_t binarySize = 0;
    if ((header.n_regions * 8) > 0x80)
    {
        throw std::runtime_error("Memory region count out of bounds");
    }
}

auto TYTFW::SupportsFirmwareFile(const std::string &file) -> bool
{
    std::ifstream i;
    i.open(file, i.binary);
    if (i.is_open())
    {
        auto header = ReadHeader(i);
        i.close();

        CheckHeader(header);

        return true;
    }
    else
    {
        throw std::runtime_error("Can't open firmware file");
    }
}

auto TYTFW::GetRadioModel() const -> const std::string
{
    return GetRadioFromMagic(counterMagic);
}

auto TYTFW::Decrypt() -> void
{
    ApplyXOR();
}

auto TYTFW::Encrypt() -> void
{
    ApplyXOR();
}

auto TYTFW::ApplyXOR() -> void
{
    const unsigned char *xor_model = nullptr;
    uint32_t xor_len = 1024; //TODO: Change this


    auto model = GetRadioFromMagic(counterMagic);
    for(const auto& xor_radio : tyt::cipher::All) {
        if(xor_radio.first == model) {
            xor_model = xor_radio.second;
            break;
        }
    }
    
    if(xor_model == nullptr)
    {
        throw std::runtime_error("No cipher found");
    }

    for (auto z = 0; z < data.size(); z++)
    {
        data[z] = data[z] ^ xor_model[z % xor_len];
    }
}