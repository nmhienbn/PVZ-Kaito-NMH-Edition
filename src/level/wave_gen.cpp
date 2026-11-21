#include "wave_gen.hpp"
#include "level/zombies_registry.hpp"
#include <stdexcept>

// Names resolve through the single registry (zombies_registry.hpp); an
// unknown name fails with the offending name in the message instead of a
// bare std::out_of_range.
int zombie_type_from_name(const string &name)
{
    if (const ZombieDef *def = zombie_def_by_key(name))
        return def->type;
    throw out_of_range("unknown zombie type name '" + name + "'");
}

string zombie_type_name(int type)
{
    if (const ZombieDef *def = zombie_def_by_type(type))
        return def->key;
    return "unknown";
}

WaveGenSpec wave_spec_from_json(const json &jzombies)
{
    WaveGenSpec spec;
    spec.total_waves = jzombies["total_waves"];
    spec.point_multiplier = jzombies.value("point_multiplier", 1);
    spec.flag_waves = jzombies.value("flag_waves", true);
    if (jzombies.count("intro_force") && !jzombies["intro_force"].is_null())
        spec.intro_force_type = zombie_type_from_name(jzombies["intro_force"]["type"]);
    for (const auto &jpick : jzombies["roster"])
    {
        ZombiePick pick;
        pick.type = zombie_type_from_name(jpick["type"]);
        pick.cost = jpick.value("cost", 1);
        pick.weight = jpick.value("weight", 4000);
        pick.first_wave = jpick.value("first_wave", 1);
        spec.roster.push_back(pick);
    }
    return spec;
}

static void add_spawn(WavePlan &plan, int type, int count)
{
    for (auto &spawn : plan.spawns)
        if (spawn.type == type)
        {
            spawn.count += count;
            return;
        }
    plan.spawns.push_back({type, count});
}

static int count_spawn(const WavePlan &plan, int type)
{
    for (const auto &spawn : plan.spawns)
        if (spawn.type == type)
            return spawn.count;
    return 0;
}

vector<WavePlan> generate_wave_plans(const WaveGenSpec &spec, mt19937 &rng)
{
    vector<WavePlan> plans(spec.total_waves);
    int waves_per_flag = spec.total_waves < 10 ? spec.total_waves : 10;
    for (int w = 0; w < spec.total_waves; w++)
    {
        WavePlan &plan = plans[w];
        // Point budget: first playthrough w/3 + 1; the replay budget
        // w*2/5 + 1 once the adventure is done (Board.cpp:642-649; level 1-5
        // is exempt from the boost).
        int points = spec.replay_budget ? w * 2 / 5 + 1 : w / 3 + 1;

        // Flag wave: forced parade, then the point bonus.
        if (spec.flag_waves && w % waves_per_flag == waves_per_flag - 1)
        {
            plan.has_flag = true;
            plan.spawns.push_back({FLAG_TYPE, 1});
            if (int forced = min(points, 8))
                plan.spawns.push_back({NORMAL_TYPE, forced});
            points = (int)(points * 2.5);
        }

        // Per-level multiplier (level 5 x4, level 10 x3).
        points *= spec.point_multiplier;

        // Forced introduction of a new zombie type
        // (repvz Board.cpp:736-759): digger/balloon appear on wave 7 (1-based)
        // or the final wave; other intro types on the middle wave and the
        // final wave.
        if (spec.intro_force_type != -1)
        {
            bool seventh_wave =
                spec.intro_force_type == DIGGER_TYPE ||
                spec.intro_force_type == BALLOON_TYPE;
            if ((seventh_wave ? w + 1 == 7 : w == spec.total_waves / 2) ||
                w == spec.total_waves - 1)
                add_spawn(plan, spec.intro_force_type, 1);
        }

        // Final wave: at least one of every allowed type.
        if (w == spec.total_waves - 1)
            for (const auto &pick : spec.roster)
                if (w + 1 >= pick.first_wave && count_spawn(plan, pick.type) == 0)
                    add_spawn(plan, pick.type, 1);

        // Spend the remaining budget on weighted-random roster picks.
        plan.points = points;
        while (points > 0)
        {
            int total_weight = 0;
            for (const auto &pick : spec.roster)
                if (w + 1 >= pick.first_wave && pick.cost <= points)
                    total_weight += pick.weight;
            if (total_weight == 0)
                break;
            int roll = (int)(rng() % total_weight);
            for (const auto &pick : spec.roster)
            {
                if (w + 1 < pick.first_wave || pick.cost > points)
                    continue;
                if (roll < pick.weight)
                {
                    add_spawn(plan, pick.type, 1);
                    points -= pick.cost;
                    break;
                }
                roll -= pick.weight;
            }
        }
    }
    return plans;
}

void dump_wave_plans(ostream &out, int level_num, const string &name, const vector<WavePlan> &plans)
{
    out << name << " (level " << level_num << ", " << plans.size() << " waves)\n";
    int total = 0;
    for (size_t w = 0; w < plans.size(); w++)
    {
        const WavePlan &plan = plans[w];
        out << "  wave " << w + 1 << (plan.has_flag ? " [FLAG]" : "")
            << " pts=" << plan.points << ": ";
        for (const auto &spawn : plan.spawns)
        {
            out << zombie_type_name(spawn.type) << " x" << spawn.count << "  ";
            total += spawn.count;
        }
        out << '\n';
    }
    out << "  total zombies: " << total << '\n';
}
