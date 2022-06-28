#include <algorithm>

#include <Agro/application.hpp>
#include <Agro/controls/group_box.hpp>
#include <Agro/controls/tree_view.hpp>
#include <Agro/controls/combo_box.hpp>
#include <Agro/controls/spin_box.hpp>

#include "data.hpp"
#include "process.hpp"
#include "game.hpp"

Tree<ItemHidden>* createItemBoxModel(TreeView<ItemHidden> *tv, std::vector<Item> &item_list, HashMap<u32, String> &item_names) {
    Tree<ItemHidden> *model = new Tree<ItemHidden>();
    Cells combo_box_item_list;
    {
        usize index = combo_box_item_list.append(new TextCellRenderer("N/A"));
        for (Item &item : item_list) {
            if (item.id == 67108864) { item.combo_index = index; }
        }
    }
    for (usize i = 0; i < item_names.capacity; i++) {
        auto &entry = item_names.entries[i];
        if (entry) {
            if (!entry.value.startsWith("I_") && entry.key > 67108865) {
                usize index = combo_box_item_list.append(new TextCellRenderer(entry.value));
                auto result = std::find_if(item_list.begin(), item_list.end(), [&](Item item) {
                    return item.name == entry.value;
                });
                if (result != item_list.end()) {
                    result->combo_index = index;
                }
            }
        }
    }
    for (u32 i = 0; i < item_list.size(); i++) {
        Item &item = item_list[i];
        ComboBox *combo_box = new ComboBox(
            [](String query, CellRenderer *cell) -> bool {
                return String(((TextCellRenderer*)cell)->text).toLower().find(query.toLower()) ? true : false;
            },
            combo_box_item_list,
            Size(0, 200)
        );
        combo_box->setCurrent(item.combo_index);
        combo_box->onItemSelected.addEventListener([&, i](Widget *widget, CellRenderer *cell, i32 index) {
            String item_name = ((TextCellRenderer*)cell)->text;
            Item &item = item_list[i];
            TreeNode<ItemHidden> *node = tv->model()->roots[i];
            for (usize j = 0; j < item_names.capacity; j++) {
                auto &entry = item_names.entries[j];
                if (entry && entry.value == item_name) {
                    ((TextCellRenderer*)node->columns[0])->text = toString(entry.key);
                    item.id = entry.key;
                    item.name = entry.value;
                    item.combo_index = index;
                    node->hidden->id = item.id;
                    node->hidden->count = item.count;
                    node->hidden->combo_index = item.combo_index;
                    node->hidden->added_order = item.added_order;
                    break;
                }
            }
        });
        TextEdit *count_edit = new TextEdit(toString(item.count));
        count_edit->onTextChanged.addEventListener([=, &item_list]() {
            String text = count_edit->text();
            i32 result = atoi(text.data());
            if (result || text == "0") {
                item_list[i].count = result;
            } else {
                warn(String::format("Unable to convert count to number: '%s'!", text));
            }
        });
        model->append(nullptr, new TreeNode<ItemHidden>({
            new TextCellRenderer(toString(item.id)),
            combo_box,
            count_edit,
        }, new ItemHidden(item.id, item.count, item.combo_index, item.added_order)));
    }
    return model;
}

int main(int argc, char **argv) {
    Process process = Process().open(MONSTER_HUNTER_RISE_EXE);
    Game game = Game(process);
    std::vector<Item> item_list;
    std::vector<Charm> charm_list;
    Cells skills;
    for (String &s : skill_names_non_null) {
        skills.append(new TextCellRenderer(s));
    }

    Application *app = Application::get();
        Window *win = app->mainWindow();
        win->resize(800, 600);
        win->center();
        win->setTitle("Monster Hunter Rise Charm and Item Box Editor");
        delete win->mainWidget();
        win->setMainWidget(new Box(Align::Horizontal));

        Box *left = new Box(Align::Vertical);
            GroupBox *charm_editor = new GroupBox(Align::Vertical, "Charm Editor");
                Box *charm_button_box = new Box(Align::Horizontal);
                    Button *reload_charms = new Button("Reload Charms");
                    charm_button_box->append(reload_charms);
                    Button *add_new_charm = new Button("Add New Charm");
                    charm_button_box->append(add_new_charm);
                charm_editor->append(charm_button_box);

                charm_editor->append(new Label("Charms"));
                DropDown *charm_picker = new DropDown(Cells(), Size(0, 200));
                charm_editor->append(charm_picker);
                charm_editor->append(new Label("Rarity"));
                DropDown *rarity_picker = new DropDown({
                    new TextCellRenderer("1"), new TextCellRenderer("2"), new TextCellRenderer("3"),
                    new TextCellRenderer("4"), new TextCellRenderer("5"), new TextCellRenderer("6"),
                    new TextCellRenderer("7"), new TextCellRenderer("4N"), new TextCellRenderer("3K"),
                    new TextCellRenderer("2V"), new TextCellRenderer("2L")
                });
                charm_editor->append(rarity_picker);
                charm_editor->append(new Label("Skill 1"));
                Box *skill_1 = new Box(Align::Horizontal);
                    ComboBox *skill_picker_1 = new ComboBox(
                        [](String query, CellRenderer *cell) -> bool {
                            return String(((TextCellRenderer*)cell)->text).toLower().find(query.toLower()) ? true : false;
                        },
                        skills,
                        Size(0, 200)
                    );
                    skill_1->append(skill_picker_1);
                    SpinBox *skill_1_level = new SpinBox();
                    skill_1->append(skill_1_level, Fill::Vertical);
                charm_editor->append(skill_1);
                charm_editor->append(new Label("Skill 2"));
                Box *skill_2 = new Box(Align::Horizontal);
                    ComboBox *skill_picker_2 = new ComboBox(
                        [](String query, CellRenderer *cell) -> bool {
                            return String(((TextCellRenderer*)cell)->text).toLower().find(query.toLower()) ? true : false;
                        },
                        skills,
                        Size(0, 200)
                    );
                    skill_2->append(skill_picker_2);
                    SpinBox *skill_2_level = new SpinBox();
                    skill_2->append(skill_2_level, Fill::Vertical);
                charm_editor->append(skill_2);

                Cells slot_capacity = {
                    new TextCellRenderer("0"), new TextCellRenderer("1"),
                    new TextCellRenderer("2"), new TextCellRenderer("3"),
                };
                charm_editor->append(new Label("Slots"));
                Box *slots = new Box(Align::Horizontal);
                    DropDown *slot_picker_1 = new DropDown(slot_capacity);
                    slots->append(slot_picker_1);
                    DropDown *slot_picker_2 = new DropDown(slot_capacity);
                    slots->append(slot_picker_2);
                    DropDown *slot_picker_3 = new DropDown(slot_capacity);
                    slots->append(slot_picker_3);
                    Button *save_charm = new Button("Save Charm");
                    slots->append(save_charm, Fill::Vertical);
                charm_editor->append(slots);
            left->append(charm_editor, Fill::Vertical);
            GroupBox *other = new GroupBox(Align::Vertical, "Other");
                Button *reload_other = new Button("Reload Zenny and Points");
                other->append(reload_other);
                other->append(new Label("Zenny"));
                Box *zenny_box = new Box(Align::Horizontal);
                    SpinBox *zenny_edit = new SpinBox(0, 200);
                    zenny_box->append(zenny_edit, Fill::Vertical);
                    Button *zenny_save = new Button("Save");
                    zenny_save->onMouseDown.addEventListener([&](Widget *widget, MouseEvent event) {
                        String text = zenny_edit->text();
                        i32 result = atoi(text.data());
                        if (result || text == "0") {
                            game.setZenny(result);
                        } else {
                            warn(String::format("Unable to convert count to number: '%s'!", text));
                        }
                    });
                    zenny_box->append(zenny_save, Fill::Vertical);
                other->append(zenny_box);
                other->append(new Label("Points"));
                Box *points_box = new Box(Align::Horizontal);
                    SpinBox *points_edit = new SpinBox(0, 200);
                    points_box->append(points_edit, Fill::Vertical);
                    Button *points_save = new Button("Save");
                    points_save->onMouseDown.addEventListener([&](Widget *widget, MouseEvent event) {
                        String text = points_edit->text();
                        i32 result = atoi(text.data());
                        if (result || text == "0") {
                            game.setPoints(result);
                        } else {
                            warn(String::format("Unable to convert count to number: '%s'!", text));
                        }
                    });
                    points_box->append(points_save, Fill::Vertical);
                other->append(points_box);
            left->append(other, Fill::Both);
        win->append(left, Fill::Vertical);
        GroupBox *right = new GroupBox(Align::Vertical, "Item Box Editor");
            Box *item_button_box = new Box(Align::Horizontal);
                Button *reload_item_box = new Button("Reload Item Box");
                item_button_box->append(reload_item_box);
                Button *save_item_box = new Button("Save Item Box");
                save_item_box->onMouseDown.addEventListener([&](Widget *widget, MouseEvent event) {
                    game.setItemBox(item_list);
                });
                item_button_box->append(save_item_box);
            right->append(item_button_box);
            TreeView<ItemHidden> *tv = new TreeView<ItemHidden>();
                tv->append(new Column<ItemHidden>("ID"));
                Column<ItemHidden> *col = new Column<ItemHidden>("Name");
                col->setExpand(true);
                tv->append(col);
                tv->append(new Column<ItemHidden>("Count"));
                Tree<ItemHidden> *model = new Tree<ItemHidden>();
                tv->setModel(model);
            right->append(tv, Fill::Both);
        win->append(right, Fill::Both);

        reload_charms->onMouseDown.addEventListener([&](Widget *widget, MouseEvent event) {
            game.getCharms(charm_list);
            assert(charm_list.size() && "No charms found in the equipment box!");
            Cells charms;
            for (Charm &charm : charm_list) {
                charms.append(new TextCellRenderer(game.formatCharm(charm, skill_names)));
            }
            charm_picker->m_list->m_items = charms;
            charm_picker->setCurrent(0);
        });

        add_new_charm->onMouseDown.addEventListener([&](Widget *widget, MouseEvent event) {
            Charm charm;
            charm.index = game.findNextEmptyEquipmentSlot();
            charm_list.push_back(charm);
            charm_picker->appendItem(new TextCellRenderer(game.formatCharm(charm, skill_names)));
            game.setCharms(charm_list);
            reload_charms->activate();
            charm_picker->setCurrent(charm_list.size() - 1);
        });

        charm_picker->onItemSelected.addEventListener([&](Widget *widget, CellRenderer *cell, i32 index) {
            Charm charm = charm_list[index];
            rarity_picker->setCurrent(charm.rarity - RARITY_1);
            skill_picker_1->setCurrent((usize)(std::find(skill_names_non_null.begin(), skill_names_non_null.end(), skill_names[charm.skill_1]) - skill_names_non_null.begin()));
            skill_1_level->setText(toString(charm.skill_1_level));
            skill_picker_2->setCurrent((usize)(std::find(skill_names_non_null.begin(), skill_names_non_null.end(), skill_names[charm.skill_2]) - skill_names_non_null.begin()));
            skill_2_level->setText(toString(charm.skill_2_level));
            slot_picker_1->setCurrent(charm.slots[0]);
            slot_picker_2->setCurrent(charm.slots[1]);
            slot_picker_3->setCurrent(charm.slots[2]);
        });

        save_charm->onMouseDown.addEventListener([&](Widget *widget, MouseEvent event) {
            Charm &charm = charm_list[charm_picker->current()];
            charm.rarity = rarity_picker->current() + RARITY_1;
            charm.skill_1 = (usize)(
                std::find(
                    skill_names.begin(),
                    skill_names.end(),
                    ((TextCellRenderer*)skill_picker_1->getItem(skill_picker_1->current()))->text
                ) - skill_names.begin()
            );
            charm.skill_2 = (usize)(
                std::find(
                    skill_names.begin(),
                    skill_names.end(),
                    ((TextCellRenderer*)skill_picker_2->getItem(skill_picker_2->current()))->text
                ) - skill_names.begin()
            );
            charm.skill_1_level = skill_1_level->value().unwrap();
            charm.skill_2_level = skill_2_level->value().unwrap();
            charm.level_1_slots = charm.level_2_slots = charm.level_3_slots = 0;
            switch (
                atoi(((TextCellRenderer*)slot_picker_1->getItem(slot_picker_1->current()))->text.data())
            ) {
                case 1: charm.level_1_slots++; break;
                case 2: charm.level_2_slots++; break;
                case 3: charm.level_3_slots++; break;
                default: break;
            };
            switch (
                atoi(((TextCellRenderer*)slot_picker_2->getItem(slot_picker_2->current()))->text.data())
            ) {
                case 1: charm.level_1_slots++; break;
                case 2: charm.level_2_slots++; break;
                case 3: charm.level_3_slots++; break;
                default: break;
            };
            switch (
                atoi(((TextCellRenderer*)slot_picker_3->getItem(slot_picker_3->current()))->text.data())
            ) {
                case 1: charm.level_1_slots++; break;
                case 2: charm.level_2_slots++; break;
                case 3: charm.level_3_slots++; break;
                default: break;
            };
            game.setCharms(charm_list);
            reload_charms->activate();
            charm_picker->setCurrent(charm_list.size() - 1);
        });

        reload_other->onMouseDown.addEventListener([&](Widget *widget, MouseEvent event) {
            zenny_edit->setText(toString(game.getZenny()));
            points_edit->setText(toString(game.getPoints()));
        });

        reload_item_box->onMouseDown.addEventListener([&](Widget *widget, MouseEvent event) {
            game.getItemBox(item_list, item_names);
            tv->setModel(createItemBoxModel(tv, item_list, item_names));
        });

        reload_charms->activate();
        reload_item_box->activate();
        reload_other->activate();

        *win->m_state = State();

    app->run();
    return 0;
}
