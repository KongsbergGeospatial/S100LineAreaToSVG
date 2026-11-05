// S100LineAreaToSVG.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <filesystem>
#include <string>
#include <unordered_map>

#include "pugixml.hpp"

void HandleArea(std::filesystem::path file)
{
    pugi::xml_document doc;
    auto pr = doc.load_file(file.c_str());

    pugi::xml_node root = doc.root();
    auto symFill = root.child("af:symbolFill");
    if (!symFill)
        return;

    double vxy[4];
    vxy[0] = atof(symFill.child("v1").child("x").child_value());
    vxy[1] = atof(symFill.child("v1").child("y").child_value());
    vxy[2] = atof(symFill.child("v2").child("x").child_value());
    vxy[3] = atof(symFill.child("v2").child("y").child_value());

    std::string symPath = file.parent_path().parent_path().string() + "/Symbols/" + symFill.child("symbol").attribute("reference").as_string() + ".svg";
    pugi::xml_document svgDoc;
    svgDoc.load_file(symPath.c_str());

    std::string outputPath = file.parent_path().string() + "/" + file.stem().string() + ".svg";
    pugi::xml_document svgOut;
    auto outRoot1 = svgOut.append_child(pugi::node_declaration);
    outRoot1.set_name("xml");
    outRoot1.append_attribute("version").set_value("1.0");
    outRoot1.append_attribute("encoding").set_value("UTF-8");
    auto outRoot2 = svgOut.append_child(pugi::node_declaration);
    outRoot2.set_name("xml-stylesheet");
    outRoot2.append_attribute("href").set_value("../Symbols/SvgStyle.css");
    outRoot2.append_attribute("type").set_value("text/css");
    auto outRoot3 = svgOut.append_child("svg");
    outRoot3.append_attribute("xmlns").set_value("http://www.w3.org/2000/svg");
    outRoot3.append_attribute("version").set_value("1.2");
    outRoot3.append_attribute("baseProfile").set_value("tiny");
    outRoot3.append_attribute("xml:space").set_value("preserve");
    outRoot3.append_attribute("shape-rendering").set_value("geometricPrecision");
    outRoot3.append_attribute("fill-rule").set_value("evenodd");
    std::string attrScratch;
    attrScratch = std::to_string(vxy[0]) +"mm";
    outRoot3.append_attribute("width").set_value(attrScratch.c_str());
    attrScratch = std::to_string(vxy[3] * 2) + "mm";
    outRoot3.append_attribute("height").set_value(attrScratch.c_str());
    attrScratch = "0 -" + std::to_string(vxy[3] / 2) +" " +std::to_string(vxy[0]) +" " + std::to_string(vxy[3] * 2);
    outRoot3.append_attribute("viewBox").set_value(attrScratch.c_str());
    auto group = outRoot3.append_child("defs").append_child("g");
    group.append_attribute("id").set_value("my-symbol");

    for (pugi::xml_node x_def = svgDoc.child("svg").first_child(); x_def; x_def = x_def.next_sibling())
    {
        if (strcmp(x_def.name(), "circle") == 0)
            group.append_copy(x_def);
        else if (strcmp(x_def.name(),"path") == 0)
            group.append_copy(x_def);
    }
    for (int i = 0; i < 4; ++i)
    {
        auto node = outRoot3.append_child("use");
        node.append_attribute("href").set_value("#my-symbol");
        switch (i)
        {
        case 0:
            node.append_attribute("x").set_value(0);
            node.append_attribute("y").set_value(vxy[1]);
            break;
        case 1:
            node.append_attribute("x").set_value(vxy[0]);
            node.append_attribute("y").set_value(vxy[1]);
            break;
        case 2:
            node.append_attribute("x").set_value(vxy[2]);
            node.append_attribute("y").set_value(vxy[3]);
            break;
        case 3:
            node.append_attribute("x").set_value(vxy[2] + vxy[0]);
            node.append_attribute("y").set_value(vxy[3]);
            break;
        }
    }
    svgOut.save_file(outputPath.c_str());
}

struct SvgBox
{
    double x,y,xmax,ymax;
};
void HandleLine(std::filesystem::path file)
{
    pugi::xml_document doc;
    auto pr = doc.load_file(file.c_str());

    pugi::xml_node root = doc.root();
    auto symFill = root.child("ls:lineStyle");
    if (!symFill)
        return;

    std::string outputPath = file.parent_path().string() + "/" + file.stem().string() + ".svg";
    pugi::xml_document svgOut;
    auto outRoot1 = svgOut.append_child(pugi::node_declaration);
    outRoot1.set_name("xml");
    outRoot1.append_attribute("version").set_value("1.0");
    outRoot1.append_attribute("encoding").set_value("UTF-8");
    auto outRoot2 = svgOut.append_child(pugi::node_declaration);
    outRoot2.set_name("xml-stylesheet");
    outRoot2.append_attribute("href").set_value("../Symbols/SvgStyle.css");
    outRoot2.append_attribute("type").set_value("text/css");
    auto outRoot3 = svgOut.append_child("svg");
    outRoot3.append_attribute("xmlns").set_value("http://www.w3.org/2000/svg");
    outRoot3.append_attribute("version").set_value("1.2");
    outRoot3.append_attribute("baseProfile").set_value("tiny");
    outRoot3.append_attribute("xml:space").set_value("preserve");
    outRoot3.append_attribute("shape-rendering").set_value("geometricPrecision");
    outRoot3.append_attribute("fill-rule").set_value("evenodd");

    SvgBox box = {};
    std::unordered_map<std::string,bool> symbolsDefed;

    double intervalLength = 0;
    double penW = 0;
    std::string color;
    intervalLength = atof(symFill.child("intervalLength").child_value());
    penW = symFill.child("pen").attribute("width").as_double();
    color = symFill.child("pen").child("color").child_value();

    box.x = 0;
    box.xmax = intervalLength;
    box.y = -penW / 2.0;
    box.ymax = penW / 2.0;

    auto x_defs = outRoot3.append_child("defs");

    for (auto x_node = symFill.first_child(); x_node; x_node = x_node.next_sibling())
    {
        if (strcmp(x_node.name(), "dash") == 0)
        {
            double start = atof(x_node.child("start").child_value());
            double length = atof(x_node.child("length").child_value());
            auto working = outRoot3.append_child("path");
            std::string pathStr = " M " + std::to_string(start) + ",0 L " + std::to_string(start + length) + ",0";
            working.append_attribute("d").set_value(pathStr.c_str());
            std::string classStr = "s1 f0 s" + color;
            working.append_attribute("class").set_value(classStr.c_str());
            working.append_attribute("stroke-width").set_value(penW);
        }
        else if (strcmp(x_node.name(), "symbol") == 0)
        {
            const char* symName = x_node.attribute("reference").as_string();
            if (!symbolsDefed[symName])
            {
                std::string symPath = file.parent_path().parent_path().string() + "/Symbols/" + symName + ".svg";
                pugi::xml_document svgDoc;
                svgDoc.load_file(symPath.c_str());
                auto group = x_defs.append_child("g");
                group.append_attribute("id").set_value(symName);

                const char* viewBoxStr = svgDoc.child("svg").attribute("viewBox").as_string();
                SvgBox viewBox;
                if (std::sscanf(viewBoxStr, "%lf %lf %lf %lf", &viewBox.x, &viewBox.y, &viewBox.xmax, &viewBox.ymax))
                {
                    if (box.y > viewBox.y)
                        box.y = viewBox.y;
                    if (box.ymax < viewBox.y + viewBox.ymax)
                        box.ymax = viewBox.y + viewBox.ymax;
                }

                for (pugi::xml_node x_def = svgDoc.child("svg").first_child(); x_def; x_def = x_def.next_sibling())
                {
                    if (strcmp(x_def.name(), "circle") == 0)
                        group.append_copy(x_def);
                    else if (strcmp(x_def.name(), "path") == 0)
                        group.append_copy(x_def);
                }
                symbolsDefed[symName] = 1;
            }
            auto node = outRoot3.append_child("use");
            std::string id = std::string("#") + symName;
            node.append_attribute("href").set_value(id.c_str());
            node.append_attribute("x").set_value(atof(x_node.child("position").child_value()));
            node.append_attribute("y").set_value(0);
        }
    }

    {
        std::string scratchStr;
        scratchStr = std::to_string(box.xmax) +"mm";
        outRoot3.append_attribute("width").set_value(scratchStr.c_str());
        scratchStr = std::to_string(box.ymax - box.y) + "mm";
        outRoot3.append_attribute("height").set_value(scratchStr.c_str());
        scratchStr = "0 " + std::to_string(box.y) + " " + std::to_string(box.xmax) + " " + std::to_string(box.ymax - box.y);
        outRoot3.append_attribute("viewBox").set_value(scratchStr.c_str());
    }
    svgOut.save_file(outputPath.c_str());
}

int main(int argc, char* argv[])
{
    std::filesystem::path root = argv[1];
    std::filesystem::path areaRoot = root / R"(AreaFills/)";
    for (const auto& entry : std::filesystem::directory_iterator(areaRoot))
    {
        if (entry.path().extension().compare(".xml") == 0)
        {
            HandleArea(entry.path());
        }
    }
    std::filesystem::path lineRoot = root / R"(LineStyles/)";
    for (const auto& entry : std::filesystem::directory_iterator(lineRoot))
    {
        if (entry.path().extension().compare(".xml") == 0)
        {
            HandleLine(entry.path());
        }
    }
}
