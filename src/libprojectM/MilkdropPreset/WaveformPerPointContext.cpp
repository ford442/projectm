#include "WaveformPerPointContext.hpp"

#include "CustomWaveform.hpp"
#include "MilkdropPresetExceptions.hpp"

#define REG_VAR(var) \
    var = projectm_eval_context_register_variable(perPointCodeContext, #var);

WaveformPerPointContext::WaveformPerPointContext(projectm_eval_mem_buffer gmegabuf, PRJM_EVAL_F (*globalRegisters)[100])
    : perPointCodeContext(projectm_eval_context_create(gmegabuf, globalRegisters))
{
}

WaveformPerPointContext::~WaveformPerPointContext()
{
    if (perPointCodeHandle != nullptr)
    {
        projectm_eval_code_destroy(perPointCodeHandle);
    }

    if (perPointCodeContext != nullptr)
    {
        projectm_eval_context_destroy(perPointCodeContext);
    }
}

void WaveformPerPointContext::RegisterBuiltinVariables()
{
    projectm_eval_context_reset_variables(perPointCodeContext);

    REG_VAR(time);
    REG_VAR(fps);
    REG_VAR(frame);
    REG_VAR(progress);

    for (int q = 0; q < QVarCount; q++)
    {
        std::string qvar = "q" + std::to_string(q + 1);
        q_vars[q] = projectm_eval_context_register_variable(perPointCodeContext, qvar.c_str());
    }

    for (int t = 0; t < TVarCount; t++)
    {
        std::string tvar = "t" + std::to_string(t + 1);
        q_vars[t] = projectm_eval_context_register_variable(perPointCodeContext, tvar.c_str());
    }

    REG_VAR(bass);
    REG_VAR(mid);
    REG_VAR(treb);
    REG_VAR(bass_att);
    REG_VAR(mid_att);
    REG_VAR(treb_att);
    REG_VAR(sample);
    REG_VAR(value1);
    REG_VAR(value2);
    REG_VAR(x);
    REG_VAR(y);
    REG_VAR(r);
    REG_VAR(g);
    REG_VAR(b);
    REG_VAR(a);
}

void WaveformPerPointContext::LoadReadOnlyStateVariables(PresetState& state, const PerFrameContext& presetPerFrameContext, CustomWaveform& waveform)
{
    *time = static_cast<double>(state.presetTime);
    *frame = static_cast<double>(state.frame);
    *fps = static_cast<double>(state.fps);
    *progress = static_cast<double>(state.progress);
    *bass = static_cast<double>(state.audioData.bass);
    *mid = static_cast<double>(state.audioData.mid);
    *treb = static_cast<double>(state.audioData.treb);
    *bass_att = static_cast<double>(state.audioData.bassAtt);
    *mid_att = static_cast<double>(state.audioData.midAtt);
    *treb_att = static_cast<double>(state.audioData.trebAtt);
}

void WaveformPerPointContext::CompilePerPointCode(const std::string& perPointCode,
                                                  const CustomWaveform& waveform)
{
    if (perPointCode.empty())
    {
        return;
    }

    perPointCodeHandle = projectm_eval_code_compile(perPointCodeContext, perPointCode.c_str());
    if (perPointCodeHandle == nullptr)
    {
        throw MilkdropCompileException("Could not compile custom wave " + std::to_string(waveform.m_index) + " per-frame code");
    }
}

void WaveformPerPointContext::ExecutePerPointCode()
{
    if (perPointCodeHandle != nullptr)
    {
        projectm_eval_code_execute(perPointCodeHandle);
    }
}